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
class [[nodiscard]] Result { ... };   // Wraps an std::expected<T, Error>.
```

`Result<T>` is a class, not an alias, so everything you do with one is a method and chains off the call:

```cpp
Blob data = reader.read(filename).orThrow();                                  // CLI tools & tests.
Blob table = engine->resources()->eventsData("dsft.bin").mustSucceed();       // Startup invariants.
MM_TRY(Font font, oef::decode(data).withContext("while loading '{}'", name)); // Propagation with context.
discard(ufs->remove(path));                                                   // Explicit "don't care".
```

`Error` (`Utility/Error/Error.h`) is constructed exactly like `Exception` was — a format string and its arguments:

```cpp
return fail("Invalid PCX version '{}' in '{}'", header->version, data.displayPath());
```

so **we keep the error messages we like**. `Error` additionally supports a *chain of context frames*, added via
`.withContext(...)` as the error travels up. Frames are joined with `": "`, outermost first:

```
Couldn't load texture 'lava': Cannot decode LOD entry 'bitmaps.lod/lava' as LOD image:
  Failed to read the requested number of bytes from stream 'bitmaps.lod/lava', requested 48, got 12
```

This is strictly better than what exceptions gave us. Today every `LodReader` message manually re-formats
`blob.displayPath()` into the string because there's nowhere else to put it; with context frames the inner code
reports what went wrong and the outer code says what it was doing.

`Error` is one `shared_ptr` — 16 bytes, cheap to copy and move, so `sizeof(Result<T>)` stays close to `sizeof(T)`
and the whole cost (formatting, allocating) is paid only on the error path. There's a unit test asserting this.

The class carries `[[nodiscard]]`, which means *every* function returning a `Result` is nodiscard automatically —
no per-declaration attribute to forget. Ignoring a return doesn't compile; dropping an error on purpose takes an
explicit, greppable `discard(...)` call.

### Propagation

C++ has no `?` operator and, since MSVC is a supported compiler, no statement expressions. So propagation is two
macros:

```cpp
MM_TRY(Blob data, fs->read(path));   // Declares `Blob data`, or early-returns the error.
MM_TRY(_pixels, decodePixels(data)); // Also works with an existing lvalue.
MM_TRY_VOID(reader.open(blob));      // Discards the value / for Result<void>.
```

Both use `__COUNTER__` for their temporaries. `MM_TRY_VOID` is a single statement; `MM_TRY` is not, and so needs
braces under a bodyless `if` — that's a compile error, not a silent bug, and it's documented.

We deliberately do **not** use monadic chaining (`and_then` / `transform`): nesting logic inside lambdas passed to
member calls reads terribly. Where two fallible calls feed each other, that's just two statements:

```cpp
MM_TRY(Blob raw, pGames_LOD->read(blv_filename));
MM_TRY(Blob location, lod::decodeMaybeCompressed(raw));
```

### Handling: the policies, all explicit

The point of the change isn't that errors stop happening — it's that *deciding what to do about them stops being
implicit*. Every policy is a named method:

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
Blob blob = engine->resources()->eventsData("dsft.bin").mustSucceed();

// 4. Throw. CLI tools and tests, where the top-level catch is the error handling — and TODO-marked
//    engine code that hasn't been ported yet.
LodImage lodImage = lod::decodeImage(entry).orThrow();

// 5. Drop, explicitly and greppably.
discard(ufs->remove(path));
```

`mustSucceed` routes through an installable `FatalErrorHandler`, so once there's a UI, "the game data is broken"
becomes a message box instead of a silent exit. And unlike a `throw`, it's greppable: you can audit every place the
game is still allowed to die.

`tryCatch(callable)` is the bridge in the other direction — it runs code that throws and returns a `Result`. It's
for third-party libraries we can't change (nlohmann/json, sol2, CLI11, the standard library) and for our own
internals that still throw (see §3). A callable that itself returns a `Result` is passed through without
double-wrapping, so `fail()` / `MM_TRY` and throwing calls can be mixed inside one `tryCatch` block.


## 3. The open question: how to chain `deserialize` calls

This is the one place where `Result` alone doesn't give a clean answer, so here are the options. First, the shape
of the problem — a real function from `CompositeSnapshots.cpp`:

```cpp
void deserialize(InputStream &src, IndoorDelta_MM7 *dst, ContextTag<IndoorLocation_MM7> ctx) {
    deserialize(src, &dst->header);
    deserialize(src, &dst->visibleOutlines);
    deserialize(src, &dst->faceAttributes, tags::presized(ctx->faces.size()));
    deserialize(src, &dst->decorationFlags, tags::presized(ctx->decorations.size()));
    deserialize(src, &dst->actors);
    // ...eight more lines of exactly this.
}
```

There are **195 `deserialize()` call sites** outside the serialization libraries, spread across ~15 files (about
half in `CompositeSnapshots.cpp` alone), plus a templated combinator layer (`std::vector`, `std::array`,
`std::span`, tag dispatch) that all of them go through. Failure in any read makes everything after it meaningless —
parsing is monotone — so *some* mechanism has to short-circuit the rest of the sequence and carry the error out.
Exceptions did that implicitly. The options for replacing them:

### Option A — exception island *(what the branch does today)*

The internals of `InputStream` and `Library/Binary` keep throwing, unchanged. Every *public* decoder converts at
its own boundary — `tryDeserialize(stream, dst, tags...)` is `tryCatch` around `deserialize`:

```cpp
Result<Palette> lod::decodePalette(const Blob &blob) {
    if (!detectImage(blob))
        return fail("Cannot decode LOD entry '{}' as LOD palette", blob.displayPath());

    return tryCatch([&] {
        MemoryInputStream stream(blob.data(), blob.size(), blob.displayPath());
        LodImageHeader_MM6 header;
        deserialize(stream, &header);
        stream.skipOrFail(header.dataSize);

        Palette result;
        deserialize(stream, &result);
        return result;
    });
}
```

* **Diff**: zero changes to the 195 call sites, zero to the combinator layer. Already done.
* **Bodies**: unchanged, maximally readable.
* **How errors short-circuit**: stack unwinding, contained inside the library.
* **Costs**: two mechanisms coexist; the island is permanent unless replaced; a boundary function that forgets its
  `tryCatch` leaks an exception upward (mitigated by the small number of boundaries and by the budget file pinning
  the island so it can't grow).
* **Failure semantics**: the half-deserialized object is abandoned, same as with exceptions today.

### Option B — thread `Result` through explicitly

`deserialize` returns `Result<void>`; every call site checks:

```cpp
Result<void> deserialize(InputStream &src, IndoorDelta_MM7 *dst, ContextTag<IndoorLocation_MM7> ctx) {
    MM_TRY_VOID(deserialize(src, &dst->header));
    MM_TRY_VOID(deserialize(src, &dst->visibleOutlines));
    MM_TRY_VOID(deserialize(src, &dst->faceAttributes, tags::presized(ctx->faces.size())));
    MM_TRY_VOID(deserialize(src, &dst->decorationFlags, tags::presized(ctx->decorations.size())));
    MM_TRY_VOID(deserialize(src, &dst->actors));
    // ...
}
```

* **Diff**: ~200 mechanical call-site edits, ~50 deserializer signatures, a rewrite of the templated combinator
  layer. The largest of the three by far.
* **Bodies**: every line grows an `MM_TRY_VOID(...)` wrapper, forever. For pure data-shuffling code this is real
  visual tax, but nothing is hidden.
* **How errors short-circuit**: an early return per line, in plain sight.
* **Costs**: churn and permanent verbosity. **Benefits**: exactly one error mechanism everywhere; the serialization
  layer becomes genuinely exception-free; nothing to forget at boundaries — the type system carries it.

### Option C — a parse cursor that carries the error

Move the failure state where the parse lives: not in `InputStream` (which stays a clean, general-purpose I/O
abstraction), but in a short-lived reader object created at the parse boundary:

```cpp
Result<IndoorDelta_MM7> loadDelta(const Blob &blob) {
    BlobInputStream stream(blob);
    BinaryReader src(&stream);       // Wraps the stream + holds the first Error.
    IndoorDelta_MM7 result;
    deserialize(src, &result);       // Bodies identical to today; src no-ops after the first failure.
    MM_TRY_VOID(src.check());        // The one check.
    return result;
}
```

`deserialize` overloads take `BinaryReader &` instead of `InputStream &`. The reader forwards reads until something
fails, records the first `Error`, and turns every subsequent read into a zero-filling no-op. Deserializer *bodies*
don't change at all; only the ~50 signatures swap the source type. The combinator layer is templated on the source
type, so both `InputStream` (for code that wants to keep streaming) and `BinaryReader` instantiate from one set of
templates.

* **Diff**: one new class, ~50 signature swaps, combinator layer templated on the source. Medium.
* **Bodies**: unchanged.
* **How errors short-circuit**: deferred checking — the same idea as `std::ios` failbits, but scoped to a
  stack-local object whose only job is one parse, rather than baked into a long-lived stream. `InputStream` itself
  never learns about errors.
* **Costs**: it *is* deferred error state, relocated. If the objection to stateful streams was to deferred checking
  as such, this option is out too. If the objection was to polluting a general-purpose I/O class with parse
  concerns, this is the fix.
* **Failure semantics**: better than A/B — the reader zero-fills unread output, so a failed parse yields a
  well-defined (if meaningless) object rather than a partially-written one.

### Option D — coroutines: `co_await` as the `?` operator *(chosen, and rolled out)*

C++20 coroutines can express Rust's `?` exactly, and it has the nicest syntax of all the options — deserializer
bodies keep their shape, with a keyword prefix instead of a macro wrapper:

```cpp
Result<void> deserialize(InputStream &src, IndoorDelta_MM7 *dst, ContextTag<IndoorLocation_MM7> ctx) {
    co_await deserialize(src, &dst->header);
    co_await deserialize(src, &dst->visibleOutlines);
    co_await deserialize(src, &dst->faceAttributes, tags::presized(ctx->faces.size()));
    co_await deserialize(src, &dst->decorationFlags, tags::presized(ctx->decorations.size()));
    co_await deserialize(src, &dst->actors);
}   // Implicit co_return: falling off the end reports success.

Result<IndoorDelta_MM7> loadDelta(const Blob &blob) {
    BlobInputStream stream(blob);
    IndoorDelta_MM7 result;
    co_await deserialize(stream, &result);
    co_return result;   // Always a move - coroutines never get NRVO.
}
```

`co_await someResult` either yields the value, or writes the error into the caller-side `Result` and destroys the
coroutine on the spot (running the destructors of its locals). **The machinery is now part of `Result`** — a
nested `promise_type` with `initial_suspend`/`final_suspend` both `suspend_never` (so the body runs synchronously
and the frame is freed on completion), an `await_transform` whose awaiter calls `handle.destroy()` on error, and
one extra `Result` constructor that lets `get_return_object` register the caller-side address (mandatory copy
elision guarantees it's the real one). Everything works and is unit-tested: short-circuiting, mid-body destructor
execution, coroutines awaiting coroutines, move-only payloads, and — a genuinely nice touch —
`unhandled_exception()` gives you `tryCatch` for free, so a throwing call inside a coroutine body becomes an
`Error` with zero extra syntax. From the outside a `Result` coroutine is indistinguishable from a plain function:
non-coroutine callers just call it and get a finished `Result` back.

#### Performance, measured (GCC 15, `-O2`)

The naive version heap-allocated one frame per call — GCC does no HALO frame elision in practice, MSVC rarely.
Fixed with **pooled frames**: the compiler looks up `operator new` in the promise type's scope, so a ~40-line
thread-local bump arena on the promise base is the entire change — nothing outside the machinery is affected.
These coroutines are fully synchronous (`suspend_never` on both ends), so frames alloc/free in strict LIFO order
even on the error path; a bump pointer with an LIFO assert is provably sufficient.

Deserializer-shaped microbenchmark, ten fallible calls per function:

| | ns/call | heap allocations/call |
| --- | ---: | ---: |
| `MM_TRY` version | 12.5 | 0 |
| `co_await`, naive | 25 | 1 (112-byte frame) |
| `co_await`, pooled frames | 20 | 0 |

Decomposing by varying the number of awaits per coroutine: the overhead is **~3.5–5 ns per frame** plus
**~0.5–2 ns per `co_await`**. Micro-optimizations beyond pooling (constinit arena, reference-holding awaiter,
noexcept annotations) measure as noise — the residual is frame setup/teardown and the optimizer losing
cross-statement visibility through the coroutine transformation, and that part doesn't go away on today's GCC.
Clang/AppleClang can elide frames entirely (HALO), so macOS will likely do better; MSVC is the open question and
Windows CI is the gate.

What makes this affordable in practice is *where* the frames are. Leaf reads (memcopy structs, bulk
`std::span`/vector payloads) are plain functions — no `co_await` inside, no frame. Only struct-level deserializers
that sequence multiple fallible calls become coroutines. A level load runs on the order of 10⁴–10⁵ such calls, so
the added cost is **~0.1–0.5 ms per load**, which is noise next to disk and zlib. The rule that keeps it that way:
never write a per-element or per-byte coroutine; bulk paths stay plain.

#### Footguns, documented in `Result.h` and pinned by tests

* A `Result<void>` coroutine must end with `co_return {};`. Flowing off the end is formally UB, and
  `-Werror=return-type` does **not** catch it. In practice our machinery surfaces it as an
  `"internal: coroutine ended without co_return"` error rather than garbage — a unit test pins that behavior.
* A coroutine body can't use plain `return` or `MM_TRY` — `co_await` / `co_return` replace both. `co_return`
  accepts a value, `fail(...)`, or another `Result`, and always moves (no NRVO).
* Never introduce a real suspension point — the frame arena relies on LIFO order and asserts on violations.
* Debuggability is still coroutine debuggability: locals live in frames, backtraces grow resume thunks, and ASAN
  is weak around frames. This is the price of the syntax.

### Considered and rejected

* **Monadic chaining** (`and_then` pipelines) — unreadable, ruled out (see §2).

### Decision and rollout

**Option D is the chosen direction** — the syntax won, and with pooled frames the measured cost on a realistic
load is fractions of a millisecond. The coroutine machinery is in `Result.h`, unit-tested, and ready to use; the
exception island (Option A) remains in place until the port lands, and the two coexist without friction — a
coroutine is just another way to produce a `Result`.

Rollout order for the serialization layer:

1. ~~Machinery in `Result` + tests.~~ Done.
2. ~~`Library/Binary` + `Library/Snapshots`: `deserialize` returns `Result<void>`; leaf memcopy / bulk span paths
   are plain functions (split at the overload level — a `co_await` in a *discarded* `if constexpr` branch still
   makes a coroutine); looping combinators are plain functions with `MM_TRY_VOID`, so per-element code never pays
   for a frame.~~ Done.
3. ~~The ~50 struct-level deserializers in `Engine/Snapshots` become coroutines: one `co_await` per read, one
   `co_return {};` at the end.~~ Done.
4. ~~Boundary functions drop their `tryCatch` wrappers (`lod::decode*` are now coroutines themselves), and
   `tryDeserialize` is gone — `deserialize` is the only spelling.~~ Done. Measured on LoadBench: identical to the
   exception-island implementation within noise, exactly as the microbenchmarks predicted.
5. `InputStream::readOrFail` / `skipOrFail` / `readAsBlobOrFail` still throw — today those throws are absorbed
   into an `Error` by the innermost enclosing coroutine (`unhandled_exception` doubles as `tryCatch`), which is
   correct but implicit. Converting them to `Result` is the remaining cleanup, and ratchets `src/Utility/` down.
6. `fromStream<T>` and `EvtInstruction::parse` still throw (TODO-marked bridges inside the `Engine/Evt` budget).
7. **Windows CI is the gate.** MSVC coroutine codegen is the one thing we can't measure locally; the public API
   wouldn't change if a leg of it ever needed to go back to option A.


## 4. What's been ported

The whole asset-loading pipeline, end to end, plus every one of its call sites. **All 45 throws in
`Library/{Lod,Snd,Vid,LodFormats,Image,Compression}` are gone.** The stream and binary serialization internals
still throw — that's the Option A island of §3 — but every public API below is `Result`, and exceptions don't
escape past it.

| Library | New signature |
| --- | --- |
| `zlib::uncompress` | `Result<Blob>` |
| `pcx::decode`, `png::decode`, `png::encode` | `Result<RgbaImage>` / `Result<Blob>` |
| `lod::decodeCompressedData`, `decodeCompressedPseudoImage`, `decodeMaybeCompressed` | `Result<Blob>` |
| `lod::decodePalette`, `decodeImage`, `decodeImageSize`, `decodeSprite`, `decodeFont` | `Result<...>` |
| `LodReader::open`, `LodReader::read` | `Result<void>` / `Result<Blob>` |
| `SndReader::open`, `SndReader::read` | `Result<void>` / `Result<Blob>` |
| `VidReader::open`, `VidReader::read` | `Result<void>` / `Result<Blob>` |
| `tryDeserialize(InputStream &, T *, tags...)` | `Result<void>` (boundary wrapper) |
| `tryDeserialize(const Blob &, T *, tags...)` | `Result<void>` (boundary wrapper) |
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
    return tryCatch([&] () -> Result<LodFont> {
        LodFont result;
        BlobInputStream stream(blob);
        deserialize(stream, &result._header, tags::via<LodFontHeader_MM7>);
        deserialize(stream, &result._atlas, atlasTag);
        result._pixels = stream.readAllAsBlob();
        MM_TRY_VOID(fixAndValidateFont(blob, result));
        return result;
    });
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

Each phase is independently shippable; the `.orThrow()` / `tryCatch` bridges hold the boundary in between.
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
4. **`Engine/Evt`** (7 throws) — event script decoding.
5. **`Library/Config`** (3) and **`Library/Fsm`** (3).
6. **The two `mustSucceed` calls on the map-loading path** (`Indoor.cpp`, `Outdoor.cpp`). These are not a
   serialization problem but a UX one: the game needs a "couldn't load the level, returning to the main menu" flow.
   Both sites are marked with a `TODO`.
7. **Install a real `FatalErrorHandler`** during startup so `mustSucceed` shows a message box.

Also part of the plan: **decide §3** — whether the exception island stays (Option A), or the serialization layer
gets ported via Option B or C. Everything above works the same either way.

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
check_exceptions: 94 throw statements, all within budget.
```

The budget file doubles as the migration to-do list.


## 7. Costs, honestly

* **`MM_TRY` is not a statement.** It expands to a declaration plus an `if`, so it needs braces as the body of a
  bodyless `if` or loop. This is a compile error rather than a silent bug, but it's a papercut. It's unavoidable
  without GNU statement expressions, which MSVC doesn't have.
* **Nested fallible calls need two lines.** You can't write `f(g(x))` when both return `Result`, and we've ruled
  out monadic chaining on readability grounds. Two `MM_TRY` lines it is.
* **The exception island (§3, Option A) means two mechanisms coexist** inside the serialization layer, and a
  boundary function that forgets its `tryCatch` leaks an exception. The budget file pins the island; replacing it
  outright is Options B/C.
* **`*` and `->` on a `Result` are `assert`-checked**, not release-checked — same trade-off as `std::expected`.
  Reviewer attention is the mitigation; the class-level `[[nodiscard]]` catches the more common mistake of ignoring
  the result entirely (it already caught three real ones during the port).
* **Two `tryDeserialize` overload families exist right now** with different return types — the `std::string_view`
  enum-parsing overloads return `bool`, the new binary ones return `Result<void>`. Phase 2 above unifies them.
* **The diff needs review.** Most of it is mechanical, but it touches ~80 files.


## 8. Verification

### Performance: level loading

`src/Bin/LoadBench` sweeps every map in `games.lod` — decompress + deserialize of all 76 `.blv`/`.odm` locations
and their `.dlv`/`.ddm` deltas, 66 MB of decompressed payload — and reports best-of-N per bucket. It exists so
that every step of this migration can be measured against master on the exact workload that exercises the
serialization layer hardest.

Measured on linux-aarch64, GCC 15, `-O3 -DNDEBUG`, best of 5 runs × 15 iterations (ms):

| variant | decompress | locations | deltas | total |
| --- | ---: | ---: | ---: | ---: |
| master, Release | 153.8 | 2.25 | 0.48 | 156.6 |
| branch, Release | 155.0 | 2.26 | 0.48 | 157.7 |
| master, Release + LTO | 152.8 | 2.20 | 0.48 | 155.5 |
| branch, Release + LTO | 151.5 | 2.19 | 0.47 | 154.1 |

Takeaways:

* **The branch costs nothing.** Master vs branch is within run-to-run noise in every bucket, with and without
  LTO — the `Result` boundary layer is invisible on this workload.
* **zlib decompression outweighs deserialization ~57:1** (≈154 ms vs ≈2.7 ms for the entire game's maps).
  Deserialization itself runs at memcpy speed (~24 GB/s here), because the bulk of the byte volume goes through
  the plain span/memcopy paths. This is also the budget context for the Option D rollout: even a several-percent
  regression in the deserialize buckets would be invisible behind zlib, though the measured coroutine overhead
  predicts far less than that.
* **LTO is worth ~1–2% end-to-end** (mostly zlib call inlining), equally on both variants.

Caveat: this container is linux-aarch64 with GCC only — no MSVC (`/GL /LTCG`), no AppleClang, no x86 hardware.
The tool is in the tree precisely so the remaining legs of the matrix can be run on real CI machines; Windows is
the important one, both for MSVC LTCG numbers and for coroutine codegen once the Option D port lands.

### Tests

* `OpenEnroth_UnitTest` — 419 tests, all passing (up from 408; the new ones cover `Error`, `Result`, the `MM_TRY`
  macros, `tryCatch`, `discard`, and `tryDeserialize`).
* `Run_GameTest_Headless_Parallel` — 332/332 passing against MM7 game data. No trace desynchronization, so game
  logic is bit-for-bit unchanged.
* `check_style` — clean.
