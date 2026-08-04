# Exception-free error handling

This document describes how OpenEnroth reports errors, why we moved away from exceptions, and what's left to do.

The short version: a `throw` deep inside asset decoding is a hidden `exit(1)` for the player. We replaced it with
`Result<T>`, which makes failure part of the function signature and forces the caller to decide what to do.


## 1. Where we were

Before this change there were **134 `throw` statements** in `src/` and `test/`. They broke down as follows.

| Area | Throws | Runs during gameplay? |
| --- | ---: | --- |
| Asset & format decoding (`Lod`, `Snd`, `Vid`, `LodFormats`, `Image`, `Compression`) | 45 | **Yes** |
| Test instrumentation (`Engine/Components/Trace`, `.../Control`) | 21 | No |
| Lua bindings (`Scripting`) | 14 | Yes, but caught by sol2 |
| Command-line tools (`Bin/*`) | 10 | No |
| Streams / `Blob` / `Utility` | 9 | **Yes** |
| Event script decoding (`Engine/Evt`) | 7 | **Yes** |
| Serialization primitives (`Binary`, `Serialization`, `Json`) | 7 | **Yes** |
| Savegame & map deserialization (`Engine/Snapshots`) | 5 | **Yes** |
| Config (`Library/Config`, `Preprocessor`) | 4 | **Yes** |
| `Library/Fsm` | 3 | **Yes** |
| Monster/table parsing (`Engine/Objects`) | 3 | **Yes** |
| Startup (`Application/Startup`) | 2 | No |
| `FileSystem` | 2 | **Yes** |
| Tests | 2 | No |

Against that there were only **26 `catch` clauses in `src/`** (35 including `test/`), and six of them were
top-level `catch (const std::exception &)` in a `main`. So the overwhelming majority of those 134 throws had exactly
one handler: *print the message and exit*.

Three observations drove the design.

**The failures are expected, not exceptional.** Almost every throw is "the bytes I was handed aren't what I expected."
That's a data-validation result, not an exceptional condition. Game data is 25 years old, comes in half a dozen
regional variants, and players mod it. `SndReader` already had a `try`/`catch` specifically to recover from a corrupt
zlib checksum in the GOG release of MM7; `UISaveLoad` had two more to survive broken savegame thumbnails;
`Indoor.cpp` and `Outdoor.cpp` had one each to respawn a map whose delta failed to parse. The recovery logic was
already there — it was just written backwards, as exception handling.

**Nothing in the type system said which functions could fail.** `Blob LodReader::read(...)` looks total. You have to
read the doc comment to learn it throws. Every one of the 195 `deserialize()` call sites in the engine was a
potential non-local exit that nothing marked.

**The deep call chains can't thread a return value.** `Engine/Snapshots/CompositeSnapshots.cpp` contains functions
that are 25 consecutive unconditional `deserialize(src, &dst->field)` calls. Making each of those check a return
value would triple the size of that file and bury the actual logic. This is the constraint that shaped the design —
see §3.


## 2. The core: `Result<T>` and `Error`

`Utility/Error/Result.h`:

```cpp
template<class T = void>
using Result = std::expected<T, Error>;
```

`Error` (`Utility/Error/Error.h`) is constructed exactly like `Exception` was — a format string and its arguments:

```cpp
return fail("Invalid PCX version '{}' in '{}'", header->version, data.displayPath());
```

so **we keep the error messages we like**. `Error` additionally supports a *chain of context frames*:

```cpp
MM_TRY_VOID(withContext(stream.check(), "File '{}' is not a valid SND", blob.displayPath()));
```

Frames are joined with `": "`, outermost first:

```
Couldn't load texture 'lava': Cannot decode LOD entry 'bitmaps.lod/lava' as LOD image:
  Could not read 'LodImageHeader_MM6' from binary stream 'bitmaps.lod/lava': expected 48 bytes, got only 12
```

This is strictly better than what exceptions gave us. Today every `LodReader` message manually re-formats
`blob.displayPath()` into the string because there's nowhere else to put it; with context frames the inner code
reports what went wrong and the outer code says what it was doing.

`Error` is one `shared_ptr` — 16 bytes, cheap to copy and move, so `sizeof(Result<T>)` stays close to `sizeof(T)`
and the whole cost (formatting, allocating) is paid only on the error path. There's a unit test asserting this.

### Propagation

C++ has no `?` operator and, since MSVC is a supported compiler, no statement expressions. So propagation is two
macros:

```cpp
MM_TRY(Blob data, fs->read(path));   // Declares `Blob data`, or early-returns the error.
MM_TRY(_pixels, decodePixels(data)); // Also works with an existing lvalue.
MM_TRY_VOID(stream.check());         // Discards the value.
```

Both use `__COUNTER__` for their temporaries. `MM_TRY_VOID` is a single statement; `MM_TRY` is not, and so needs
braces under a bodyless `if` — that's a compile error, not a silent bug, and it's documented.

For chaining two fallible calls in one expression, `std::expected`'s monadic operations do the job and read well:

```cpp
MM_TRY(Blob deltaBlob, pGames_LOD->read(dlv_filename).and_then(lod::decodeMaybeCompressed));
```

### Handling: three explicit policies

The point of the change isn't that errors stop happening — it's that *deciding what to do about them stops being
implicit*. There are exactly three answers, and each has a name:

```cpp
// 1. Propagate. Your caller knows better than you do.
MM_TRY(LodImage image, lod::decodeImage(blob));

// 2. Degrade. The game keeps running. This is the right answer inside the game loop.
Result<RgbaImage> image = pcx::decode(save.thumbnail);
if (!image) {
    logger->debug("Couldn't decode a savegame thumbnail: {}", image.error());
    return nullptr;
}

// 3. Die, on purpose, with a message. Startup-time invariants only.
Blob blob = mustSucceed(engine->resources()->eventsData("dsft.bin"));
```

`mustSucceed` routes through an installable `FatalErrorHandler`, so once there's a UI, "the game data is broken"
becomes a message box instead of a silent exit. And unlike a `throw`, it's greppable: you can audit every place the
game is still allowed to die.

### Bridges

Two helpers convert between the worlds, both deliberately conspicuous:

* `tryCatch(callable)` — runs code that throws and returns a `Result`. For third-party libraries we can't change
  (nlohmann/json, sol2, CLI11, the standard library).
* `orThrow(result)` — turns a `Result` back into an exception. Correct and permanent in `src/Bin/*` and `test/`,
  where the top-level `catch` *is* the error handling. Anywhere in the engine, it's a `TODO` marking the migration
  frontier.


## 3. The interesting part: sticky error state on streams

Threading `Result` through the binary deserialization layer was never going to work. Here's a real function from
`CompositeSnapshots.cpp`:

```cpp
void deserialize(InputStream &src, IndoorDelta_MM7 *dst, const IndoorLocation_MM7 &ctx) {
    deserialize(src, &dst->header);
    deserialize(src, &dst->visibleOutlines);
    deserialize(src, &dst->faceAttributes, tags::presized(ctx.faces.size()));
    deserialize(src, &dst->decorationFlags, tags::presized(ctx.decorations.size()));
    deserialize(src, &dst->actors);
    // ...eight more lines of exactly this.
}
```

There are **195 `deserialize()` call sites** outside the serialization libraries, spread across ~15 files (about
half of them in `CompositeSnapshots.cpp` alone), plus a templated combinator layer
(`std::vector`, `std::array`, `std::span`, tag dispatch) that all of them go through. Putting an `MM_TRY_VOID`
around each one means 195 mechanical edits, ~15 signature changes, a rewrite of the combinators, and a permanent
tax on the readability of pure data-shuffling code.

Instead, `InputStream` grew a **sticky error state**, exactly like `std::ios::failbit`:

* The first error is stored in the stream and never overwritten — the first one is the one that explains what
  actually happened.
* Once failed, `read` / `skip` / `readAll` become no-ops returning 0.
* A failed read **zero-fills whatever it couldn't read**, so deserialized objects are always well-defined. Nothing
  downstream can trip over uninitialized memory before the error is noticed. (The existing bound check on
  size-prefixed containers means a garbage length prefix still can't turn into a huge allocation — there's a
  regression test for this.)
* `stream.check()` returns a `Result<void>` with the first error.

So the 195 call sites above change by **zero lines**. The function that owns the stream checks once:

```cpp
Result<Palette> lod::decodePalette(const Blob &blob) {
    MemoryInputStream stream(blob.data(), blob.size(), blob.displayPath());
    LodImageHeader_MM6 header;
    deserialize(stream, &header);
    stream.skipOrFail(header.dataSize);

    Palette result;
    deserialize(stream, &result);
    MM_TRY_VOID(stream.check());   // <- the one check
    return result;
}
```

The generic error message also got better in the process, because the stream knows its own display path:

```
Could not read 'LodImageHeader_MM6' from binary stream 'bitmaps.lod/lava': expected 48 bytes, got only 12
```

**Why this is safe rather than sloppy.** The objection to sticky state is "you can forget to check." Three things
answer it:

1. Parsing is monotone. Once the data is bad, everything after it is meaningless, and there is no useful action
   between the first failure and the end of the parse. Checking at each step buys nothing.
2. Failure is contained. A failed stream produces zeros, not garbage, and the checks that guard allocations still
   run. Forgetting to check produces a zero-filled object, not memory corruption.
3. The check happens at the boundary, and the boundary is exactly where `Result<T>` and `[[nodiscard]]` take over.
   `lod::decodePalette` returns `Result<Palette>`; its callers cannot ignore that.

This is the design decision most worth arguing about, so it's worth being explicit: sticky state is used **only**
for the byte-level parsing layer, where the call chains are deep and mechanical. Everything above it — every public
API — is `Result<T>`.


## 4. What's been ported

The whole asset-loading pipeline, end to end, plus every one of its call sites. **All 45 throws in
`Library/{Lod,Snd,Vid,LodFormats,Image,Compression}` are gone**, along with the ones in the stream and binary
serialization layers.

| Library | New signature |
| --- | --- |
| `zlib::uncompress` | `Result<Blob>` |
| `pcx::decode`, `png::decode`, `png::encode` | `Result<RgbaImage>` / `Result<Blob>` |
| `lod::decodeCompressedData`, `decodeCompressedPseudoImage`, `decodeMaybeCompressed` | `Result<Blob>` |
| `lod::decodePalette`, `decodeImage`, `decodeImageSize`, `decodeSprite`, `decodeFont` | `Result<...>` |
| `LodReader::open`, `LodReader::read` | `Result<void>` / `Result<Blob>` |
| `SndReader::open`, `SndReader::read` | `Result<void>` / `Result<Blob>` |
| `VidReader::open`, `VidReader::read` | `Result<void>` / `Result<Blob>` |
| `InputStream::readOrFail`, `skipOrFail` | sticky, `check()` returns `Result<void>` |
| `tryDeserialize(const Blob &, T *)` | `Result<void>` |
| `LodTextureCache::read`, `LoadCompressedTexture` | `Result<Blob>` |
| `ResourceManager::open`, `eventsData` | `Result<void>` / `Result<Blob>` |
| `IndoorLocation::Load`, `OutdoorLocation::Load` | `Result<void>` |

`throw` count: **134 → 86**, and the remainder is dominated by places where exceptions are the right answer
(14 in the Lua bindings, 21 in test instrumentation, 9 in CLI tools).

### Seven `catch` clauses became ordinary control flow

This is the part that shows the design is actually nicer to use, not just safer.

**`lod::decodeFont`** used exceptions to try two on-disk layouts:

```cpp
// Before
LodFont result;
try {
    BlobInputStream stream(blob);
    deserialize(stream, &result._header, tags::via<LodFontHeader_MM7>);
    deserialize(stream, &result._atlas, tags::via<LodFontAtlas_MM7>);
    result._pixels = stream.readAllAsBlob();
    fixAndValidateFont(blob, result);
} catch (const std::exception &e) {
    try {
        BlobInputStream stream(blob);
        deserialize(stream, &result._header, tags::via<LodFontHeader_MM7>);
        deserialize(stream, &result._atlas, tags::via<LodFontAtlas_MMX>);
        result._pixels = stream.readAllAsBlob();
        fixAndValidateFont(blob, result);
    } catch (const std::exception &) {
        throw e; // Re-throw outer exception if trying both formats failed.
    }
}
return result;

// After
auto tryDecode = [&] (auto atlasTag) -> Result<LodFont> {
    LodFont result;
    BlobInputStream stream(blob);
    deserialize(stream, &result._header, tags::via<LodFontHeader_MM7>);
    deserialize(stream, &result._atlas, atlasTag);
    result._pixels = stream.readAllAsBlob();
    MM_TRY_VOID(stream.check());
    MM_TRY_VOID(fixAndValidateFont(blob, result));
    return result;
};

Result<LodFont> result = tryDecode(tags::via<LodFontAtlas_MM7>);
if (!result)
    if (Result<LodFont> mmx = tryDecode(tags::via<LodFontAtlas_MMX>))
        return mmx;
return result; // If both layouts failed then report the error from the first one, it's the more likely one.
```

The duplicated body is gone, `throw e` (which sliced the exception, incidentally) is gone, and the fallback is
visible as a fallback.

**`Indoor.cpp` / `Outdoor.cpp`** — a `try` block wrapping 15 lines so that a bad map delta triggers a respawn:

```cpp
// Before                                  // After
try {                                      if (Result<void> deserialized = tryDeserialize(deltaBlob, &delta, tags::context(location))) {
    deserialize(blob, &delta, ...);            ...
    ...                                    } else {
} catch (const Exception &e) {                 logger->error("Failed to load '{}', respawning location: {}",
    logger->error("... {}", e.what());                        dlv_filename, deserialized.error());
    respawnInitial = true;                     respawnInitial = true;
}                                          }
```

**`UISaveLoad.cpp`** — two near-identical `try`/`catch` blocks for broken thumbnails collapsed into one small
helper with an early return.

**`SndReader::read`** and the two nested `catch`es in `lod::decodeFont` above account for the rest. Seven `catch`
clauses removed, none added outside the two bridge functions (`Error::fromCurrentException` and `tryCatch`).

The `SndReader::read` corrupt-checksum recovery path likewise became an `if`, and its fallback error now carries the
original zlib message as a context frame rather than dropping it.


## 5. Migration plan for the rest

Each phase is independently shippable; the `orThrow` / `checkOrThrow` bridges hold the boundary in between.
`CMakeModules/exception_budget.txt` is the ratchet — see §6.

1. **`Blob::fromFile` and `Library/FileSystem`** (4 throws, wide fan-out). `FileSystemException` already carries a
   `FileSystemError` code, so it maps onto `Error`'s `std::error_code` directly. Doing this removes the three
   `tryCatch([&] { return Blob::fromFile(path); })` bridges currently in `LodReader`, `SndReader` and `VidReader`.
2. **`Library/Serialization`** (4 throws). Enum and number parsing. Note that `tryDeserialize` is currently
   overloaded with two different meanings — the `std::string_view` overloads return `bool`, the new `Blob` overload
   returns `Result<void>`. Unify them on `Result<void>` in this phase.
3. **`Engine/Snapshots`** (5 throws) — savegame and map deserialization. The highest-value remaining phase: a
   corrupt savegame should show an error in the UI, not take the game down. The bridge is already marked with a
   `TODO` in `CompositeSnapshots.cpp`.
4. **`Engine/Evt`** (7 throws) — event script decoding. `EvtInstruction::parse` currently ends with an explicit
   `stream.checkOrThrow()`; converting it to `Result` is mechanical.
5. **`Library/Config`** (3) and **`Library/Fsm`** (3).
6. **The two `mustSucceed` calls on the map-loading path** (`Indoor.cpp`, `Outdoor.cpp`). These are not a
   serialization problem but a UX one: the game needs a "couldn't load the level, returning to the main menu" flow.
   Both sites are marked with a `TODO`.
7. **Install a real `FatalErrorHandler`** during startup so `mustSucceed` shows a message box.

Staying as-is, permanently:

* **`src/Scripting`** (14). sol2's contract is that a C++ exception becomes a Lua error. That's the sanctioned
  bridge, and it's already contained.
* **`src/Bin/*`** (9) and **`test/`** (2). Process exit and test failure *are* the error handling.
* **`Engine/Components/{Trace,Control}`** (21). Test instrumentation, compiled in but only reachable under
  `--instrumented`. `EngineControlState::TerminationException` is genuinely a control-flow exception (it unwinds a
  fiber), and there's no reason to change it.

### What we are *not* doing: `-fno-exceptions`

It isn't reachable, and it isn't the goal. sol2, nlohmann/json, CLI11 and the standard library itself all throw, and
`std::bad_alloc` will always exist. The objective is that **no exception ever crosses into engine code** — which is
a property we can actually enforce, and which delivers the determinism we're after.


## 6. Keeping it that way

`CMakeModules/check_exceptions.py`, wired into the existing `check_style` target, counts `throw` statements per
directory and compares them against `CMakeModules/exception_budget.txt`. Every entry has a number and a reason.

* A `throw` in a directory with no budget is an error.
* Going over a budget is an error.
* Going *under* prints a reminder to lower the number, so the ratchet only turns one way.

```
$ ninja check_exceptions
check_exceptions: 90 throw statements, all within budget.
```

The budget file doubles as the migration to-do list.


## 7. Costs, honestly

* **`MM_TRY` is not a statement.** It expands to a declaration plus an `if`, so it needs braces as the body of a
  bodyless `if` or loop. This is a compile error rather than a silent bug, but it's a papercut. It's unavoidable
  without GNU statement expressions, which MSVC doesn't have.
* **Nested fallible calls need two lines**, or `and_then`. You can't write `f(g(x))` when both return `Result`.
* **Sticky stream state can be forgotten.** See §3 for why the blast radius is a zero-filled struct rather than
  anything worse, but it is a real trade-off, deliberately taken.
* **`.value()`, `*`, and `->` on a `Result` are unchecked** in release builds, same as any `std::expected`. Reviewer
  attention is the mitigation; `[[nodiscard]]` catches the more common mistake of ignoring the result entirely.
* **Two `tryDeserialize` overload families exist right now** with different return types. Phase 2 above unifies them.
* **The diff touches 76 files** (+1660/-519). Most of it is mechanical, but it does need review.


## 8. Verification

* `OpenEnroth_UnitTest` — 412 tests, all passing (up from 408; the new ones cover `Error`, `Result`, the `MM_TRY`
  macros, the sticky stream state, and `tryDeserialize`).
* `Run_GameTest_Headless_Parallel` — 332/332 passing against MM7 game data. No trace desynchronization, so game
  logic is bit-for-bit unchanged.
* `check_style` — clean.
