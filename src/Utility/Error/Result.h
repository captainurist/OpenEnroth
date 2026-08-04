#pragma once

#include <cassert>
#include <coroutine>
#include <cstddef>
#include <expected>
#include <new>
#include <type_traits>
#include <utility>

#include "Utility/Exception.h"

#include "Error.h"

template<class T = void>
class Result;

namespace detail {
template<class T>
constexpr bool is_result_v = false;
template<class T>
constexpr bool is_result_v<Result<T>> = true;
} // namespace detail

/**
 * Return type for functions that can fail in a recoverable way.
 *
 * `Result<T>` holds either a `T`, or an `Error` explaining why the `T` couldn't be produced. It's the replacement
 * for throwing an `Exception`: instead of unwinding the stack from somewhere deep inside the engine, a function
 * makes the failure part of its signature, and the caller is forced by `[[nodiscard]]` to decide what to do
 * about it.
 *
 * Producing an error:
 * ```
 * Result<RgbaImage> pcx::decode(const Blob &data) {
 *     if (data.size() < sizeof(PcxHeader))
 *         return fail("PCX image '{}' is too small, expected at least {} bytes, got {}",
 *                     data.displayPath(), sizeof(PcxHeader), data.size());
 *     ...
 * }
 * ```
 *
 * Propagating an error – write the function as a coroutine and use `co_await`, which either produces the value or
 * ends the function right there with the error:
 * ```
 * Result<Font> loadFont(std::string_view name) {
 *     Blob data = co_await fs->read(name);
 *     Font font = co_await oef::decode(data).withContext("while loading font '{}'", name);
 *     co_await validate(font);
 *     co_return font;
 * }
 * ```
 *
 * In plain functions (hot per-element code where a coroutine frame isn't wanted, see the coroutine docs below),
 * propagate with an explicit check instead:
 * ```
 * if (Result<void> result = deserialize(src, &element); !result)
 *     return result;
 * ```
 *
 * Handling an error – pick one of the policies, explicitly:
 * ```
 * // 1. Propagate, adding context. See the coroutine example above.
 *
 * // 2. Degrade. The game keeps running with a fallback. This is the right answer inside the game loop.
 * Result<RgbaImage> image = pcx::decode(thumbnail);
 * if (!image) {
 *     logger->warning("Bad savegame thumbnail: {}", image.error());
 *     return nullptr;
 * }
 *
 * // 3. Die, loudly and on purpose. Only for things that make the game unusable, and preferably only at startup.
 * Blob icon = dfs->read("images/OpenEnroth.png").mustSucceed();
 *
 * // 4. Throw. Only in CLI tools and tests, where the top-level catch is the error handling, and in
 * //    not-yet-ported engine code, where every use is a TODO.
 * Blob data = reader.read(filename).orThrow();
 * ```
 *
 * @see Error
 */
template<class T>
class [[nodiscard]] Result {
    static_assert(!std::is_same_v<T, Error>, "Result<Error> doesn't make sense.");
 public:
    struct promise_type; // Makes `Result<T> f() { co_return ...; }` a coroutine. See the coroutine docs below.

    Result(T value) : _impl(std::move(value)) {} // NOLINT(runtime/explicit): implicit conversion is intended.
    Result(Error error) : _impl(std::unexpect, std::move(error)) {} // NOLINT(runtime/explicit): same.

    /**
     * @return                          Whether this `Result` holds a value.
     */
    [[nodiscard]] bool ok() const {
        return _impl.has_value();
    }

    // Not available for `Result<bool>` - `if (fs->exists(path))` checking whether the *call* succeeded rather
    // than whether the file exists is exactly the kind of silent bug we don't want to compile. Use `ok()` or
    // `valueOr(false)` there instead.
    [[nodiscard]] explicit operator bool() const requires (!std::is_same_v<T, bool>) {
        return ok();
    }

    [[nodiscard]] T &operator*() & {
        assert(ok());
        return *_impl;
    }

    [[nodiscard]] const T &operator*() const & {
        assert(ok());
        return *_impl;
    }

    [[nodiscard]] T &&operator*() && {
        assert(ok());
        return *std::move(_impl);
    }

    [[nodiscard]] T *operator->() {
        assert(ok());
        return &*_impl;
    }

    [[nodiscard]] const T *operator->() const {
        assert(ok());
        return &*_impl;
    }

    /**
     * @return                          The error held by this `Result`. Must not be called on a `Result` holding
     *                                  a value.
     */
    [[nodiscard]] const Error &error() const & {
        assert(!ok());
        return _impl.error();
    }

    [[nodiscard]] Error error() && {
        assert(!ok());
        return std::move(_impl).error();
    }

    /**
     * Unwraps this `Result`, throwing an `Exception` if it holds an error.
     *
     * This is for code that legitimately wants to handle errors by throwing:
     * - Command-line tools and unit tests, where the top-level `catch` is the error handling, and there is no game
     *   loop to keep running.
     * - Engine code that hasn't been converted to `Result` yet – there, every use of this method is a `TODO`.
     *
     * Never use it in the game itself. `mustSucceed` is the honest way to say "this can't fail" there.
     *
     * @return                          The value held by this `Result`.
     * @throws Exception                If this `Result` holds an error.
     */
    T orThrow() && {
        if (!ok())
            throw Exception("{}", _impl.error().message());
        return *std::move(_impl);
    }

    /**
     * Unwraps this `Result`, terminating the process through `fatalError` if it holds an error.
     *
     * This is the explicit, greppable equivalent of letting an exception escape into `main`. Use it only where
     * there is genuinely nothing to fall back to – e.g. when the game data folder turns out to be unreadable during
     * startup. Never use it on anything that runs inside the game loop.
     *
     * @return                          The value held by this `Result`.
     */
    T mustSucceed() && {
        if (!ok())
            fatalError(_impl.error());
        return *std::move(_impl);
    }

    /**
     * @param fallback                  Value to return if this `Result` holds an error.
     * @return                          The value held by this `Result`, or `fallback`.
     */
    /**
     * Explicitly discards this `Result`, error and all.
     *
     * `Result` is `[[nodiscard]]`, so simply ignoring a return value doesn't compile – we don't want errors to be
     * dropped by accident. This method is the way to drop one *on purpose*, and it's greppable:
     * ```
     * ufs->remove(path).discard(); // We don't care whether it existed.
     * ```
     */
    void discard() && {}

    [[nodiscard]] T valueOr(T fallback) && {
        if (!ok())
            return fallback;
        return *std::move(_impl);
    }

    /**
     * @param fmt                       Format string describing what the caller was doing.
     * @param args                      Format arguments.
     * @return                          This `Result`, with an additional context frame if it holds an error.
     */
    template<class... Args>
    Result withContext(fmt::format_string<Args...> fmt, Args &&... args) && {
        if (ok())
            return std::move(*this);
        return Result(std::move(_impl).error().withContext(fmt, std::forward<Args>(args)...));
    }

 private:
    friend promise_type;
    explicit Result(promise_type *promise); // Coroutine machinery only.

 private:
    std::expected<T, Error> _impl;
};

template<>
class [[nodiscard]] Result<void> {
 public:
    struct promise_type; // Makes `Result<> f() { co_return; }` a coroutine. See the coroutine docs below.

    Result() = default;
    Result(Error error) : _impl(std::unexpect, std::move(error)) {} // NOLINT(runtime/explicit): implicit is intended.

    [[nodiscard]] bool ok() const {
        return _impl.has_value();
    }

    [[nodiscard]] explicit operator bool() const {
        return ok();
    }

    [[nodiscard]] const Error &error() const & {
        assert(!ok());
        return _impl.error();
    }

    [[nodiscard]] Error error() && {
        assert(!ok());
        return std::move(_impl).error();
    }

    /**
     * Explicitly discards this `Result`, error and all.
     *
     * `Result` is `[[nodiscard]]`, so simply ignoring a return value doesn't compile – we don't want errors to be
     * dropped by accident. This method is the way to drop one *on purpose*, and it's greppable:
     * ```
     * ufs->remove(path).discard(); // We don't care whether it existed.
     * ```
     */
    void discard() && {}

    void orThrow() && {
        if (!ok())
            throw Exception("{}", _impl.error().message());
    }

    void mustSucceed() && {
        if (!ok())
            fatalError(_impl.error());
    }

    template<class... Args>
    Result withContext(fmt::format_string<Args...> fmt, Args &&... args) && {
        if (ok())
            return {};
        return Result(std::move(_impl).error().withContext(fmt, std::forward<Args>(args)...));
    }

 private:
    friend promise_type;
    explicit Result(promise_type *promise); // Coroutine machinery only.

 private:
    std::expected<void, Error> _impl;
};

/**
 * Creates an `Error` for returning from a `Result`-returning function.
 *
 * This is just a more readable synonym for constructing an `Error` in a `return` statement:
 * ```
 * return fail("Entry '{}' doesn't exist in LOD file '{}'", filename, _lod.displayPath());
 * ```
 *
 * @param fmt                           Format string.
 * @param args                          Format arguments.
 * @return                              `Error` with the formatted message.
 */
template<class... Args>
[[nodiscard]] Error fail(fmt::format_string<Args...> fmt, Args &&... args) {
    return Error(fmt, std::forward<Args>(args)...);
}


/**
 * Runs a callable, converting any exception it throws into an `Error`.
 *
 * This is the bridge from code that throws – third-party libraries (nlohmann/json, sol2, CLI11, the standard
 * library), and the parts of our own code that haven't been converted to `Result` yet. Exceptions should not travel
 * any further up the stack than this.
 *
 * The callable itself may also return a `Result` – it won't get double-wrapped, so it's OK to mix `fail()` with
 * throwing calls inside one `tryCatch` block.
 *
 * @param callable                      Callable to run.
 * @return                              Whatever `callable` returned, or the exception it threw, as an `Error`.
 */
template<class Callable>
[[nodiscard]] auto tryCatch(Callable &&callable) {
    using R = std::invoke_result_t<Callable>;
    if constexpr (detail::is_result_v<R>) {
        try {
            return std::forward<Callable>(callable)();
        } catch (...) {
            return R(Error::fromCurrentException());
        }
    } else if constexpr (std::is_void_v<R>) {
        try {
            std::forward<Callable>(callable)();
            return Result<>();
        } catch (...) {
            return Result<>(Error::fromCurrentException());
        }
    } else {
        try {
            return Result<R>(std::forward<Callable>(callable)());
        } catch (...) {
            return Result<R>(Error::fromCurrentException());
        }
    }
}

// Coroutine support: `co_await` as the `?` operator.
//
// A function returning `Result<T>` can be written as a coroutine. Inside such a function, `co_await someResult`
// either produces the value, or ends the function right there with the error - the exact equivalent of Rust's `?`,
// as a language construct instead of a macro:
//
// ```
// Result<void> deserialize(InputStream &src, IndoorDelta_MM7 *dst) {
//     co_await deserialize(src, &dst->header);
//     co_await deserialize(src, &dst->visibleOutlines);
// }
// ```
//
// These coroutines are fully synchronous: the body runs to completion (or is aborted by a failed `co_await`)
// inside the call expression, so from the outside a coroutine is indistinguishable from a regular function - any
// caller just gets a finished `Result` back. Coroutine frames are allocated from a thread-local LIFO arena, not
// the heap.
//
// Rules of the road:
// - In a `Result<T>` coroutine, `co_return` accepts anything a `Result<T>` accepts: a value, `fail(...)`, or
//   another `Result<T>`.
// - A `Result<void>` coroutine doesn't need a `co_return` at all - flowing off the end is success, and a bare
//   `co_return;` works for returning early. It can't `co_return` an error though (the language doesn't allow
//   both `return_void` and `return_value` in one promise) - to end with an error, `co_await` it:
//   `co_await fail("...")`, or `co_await someFailedResult`.
// - Flowing off the end of a *non-void* `Result<T>` coroutine is formally UB, same as forgetting `return` in a
//   regular function - except `-Werror=return-type` does NOT catch it. In practice (pinned by a unit test) the
//   caller gets the "internal: coroutine ended without co_return" error, but don't rely on it.
// - A coroutine body can't use plain `return` - propagation is what `co_await` is for.
// - Exceptions thrown inside the body are converted to an `Error` automatically, so a coroutine gets `tryCatch`
//   semantics for free.
// - An actual suspension point must never be introduced (no awaiting real async things) - the frame arena relies
//   on strict LIFO allocation order, and asserts on it.
//

namespace detail {

/**
 * Thread-local arena for `Result` coroutine frames.
 *
 * `Result` coroutines are fully synchronous, so their frames are allocated and freed in strict LIFO order, even
 * when an error aborts a coroutine mid-body - which makes a bump allocator sufficient. GCC and MSVC essentially
 * never elide coroutine frame allocations, so without this every coroutine call would be a `malloc`.
 */
class ResultCoroutineArena {
 public:
    constexpr ResultCoroutineArena() = default;

    void *allocate(std::size_t size) {
        size = align(size);
        if (_top + size > sizeof(_buffer)) [[unlikely]]
            return ::operator new(size); // Arena overflow, fall back to the heap.
        void *result = _buffer + _top;
        _top += size;
        return result;
    }

    void deallocate(void *ptr, std::size_t size) {
        if (ptr < _buffer || ptr >= _buffer + sizeof(_buffer)) [[unlikely]] {
            ::operator delete(ptr); // Was a heap fallback.
            return;
        }
        size = align(size);
        assert(static_cast<std::byte *>(ptr) == _buffer + _top - size && "Result coroutines must stay synchronous");
        _top -= size;
    }

 private:
    static std::size_t align(std::size_t size) {
        return (size + 15) & ~std::size_t(15);
    }

 private:
    alignas(16) std::byte _buffer[64 * 1024] = {}; // Zero-init lands in .tbss, so this costs nothing at runtime.
    std::size_t _top = 0;
};

inline constinit thread_local ResultCoroutineArena resultCoroutineArena;

/**
 * The awaiter behind `co_await someResult`: either produces the value, or writes the error into the enclosing
 * coroutine's caller-side `Result` and destroys the coroutine on the spot, running the destructors of its locals.
 *
 * Holds a reference - the awaited `Result` is a temporary of the `co_await` full-expression and outlives the
 * awaiter.
 */
template<class U, class Out>
struct ResultAwaiter {
    Result<U> &awaited;
    Out *out;

    bool await_ready() const noexcept { return awaited.ok(); }

    U await_resume() noexcept(std::is_nothrow_move_constructible_v<U>) { return *std::move(awaited); }

    void await_suspend(std::coroutine_handle<> handle) noexcept {
        *out = Out(std::move(awaited).error());
        handle.destroy(); // `this` lives in the frame being destroyed - not another word about it past this line.
    }
};

/**
 * The awaiter behind `co_await someError` - the "raise" idiom: unconditionally ends the coroutine with the
 * awaited `Error`. This is also the only way to end a `Result<void>` coroutine with an error, as its promise has
 * `return_void`, and the language doesn't allow `return_value` alongside it.
 */
template<class Out>
struct ErrorAwaiter {
    Error &error;
    Out *out;

    bool await_ready() const noexcept { return false; }
    void await_resume() noexcept {} // Unreachable.

    void await_suspend(std::coroutine_handle<> handle) noexcept {
        *out = Out(std::move(error));
        handle.destroy();
    }
};

template<class Out>
struct ResultAwaiter<void, Out> {
    Result<void> &awaited;
    Out *out;

    bool await_ready() const noexcept { return awaited.ok(); }
    void await_resume() noexcept {}

    void await_suspend(std::coroutine_handle<> handle) noexcept {
        *out = Out(std::move(awaited).error());
        handle.destroy();
    }
};

template<class Out>
struct ResultPromiseBase {
    Out *out = nullptr;

    static void *operator new(std::size_t size) { return resultCoroutineArena.allocate(size); }
    static void operator delete(void *ptr, std::size_t size) { resultCoroutineArena.deallocate(ptr, size); }

    // Both suspend_never: the body runs synchronously inside the call expression, and the frame is freed the
    // moment it completes.
    std::suspend_never initial_suspend() const noexcept { return {}; }
    std::suspend_never final_suspend() const noexcept { return {}; }

    void unhandled_exception() { *out = Out(Error::fromCurrentException()); }

    template<class U>
    ResultAwaiter<U, Out> await_transform(Result<U> &&awaited) noexcept {
        return {awaited, out};
    }

    ErrorAwaiter<Out> await_transform(Error &&error) noexcept {
        return {error, out};
    }
};

// This is what the caller sees if a non-void coroutine flows off the end without `co_return` - see the rules
// above.
inline const Error resultCoroutineAbandonedError("internal: coroutine ended without co_return");

} // namespace detail

template<class T>
struct Result<T>::promise_type : detail::ResultPromiseBase<Result<T>> {
    Result get_return_object() noexcept { return Result(this); }

    // Takes a Result, so `co_return value;`, `co_return fail(...);` and `co_return otherResult;` all work.
    void return_value(Result value) noexcept(std::is_nothrow_move_constructible_v<T>) {
        *this->out = std::move(value);
    }
};

struct Result<void>::promise_type : detail::ResultPromiseBase<Result<void>> {
    Result get_return_object() noexcept { return Result(this); }

    // `return_void`, not `return_value`: flowing off the end of a `Result<void>` coroutine is well-defined
    // success, and no trailing `co_return` is needed. See the rules above for how to end one with an error.
    void return_void() noexcept { *this->out = Result(); }
};

template<class T>
Result<T>::Result(promise_type *promise) : _impl(std::unexpect, detail::resultCoroutineAbandonedError) {
    promise->out = this; // Mandatory copy elision guarantees that `this` is the caller-side object.
}

inline Result<void>::Result(promise_type *promise) : _impl(std::unexpect, detail::resultCoroutineAbandonedError) {
    promise->out = this;
}
