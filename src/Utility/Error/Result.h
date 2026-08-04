#pragma once

#include <cassert>
#include <expected>
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
 * Propagating an error – use `MM_TRY` / `MM_TRY_VOID`, they early-return from the enclosing function:
 * ```
 * Result<Font> loadFont(std::string_view name) {
 *     MM_TRY(Blob data, fs->read(name));
 *     MM_TRY(Font font, oef::decode(data).withContext("while loading font '{}'", name));
 *     MM_TRY_VOID(validate(font));
 *     return font;
 * }
 * ```
 *
 * Handling an error – pick one of the policies, explicitly:
 * ```
 * // 1. Propagate, adding context. See MM_TRY above.
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
    Result(T value) : _impl(std::move(value)) {} // NOLINT(runtime/explicit): implicit conversion is intended.
    Result(Error error) : _impl(std::unexpect, std::move(error)) {} // NOLINT(runtime/explicit): same.

    /**
     * @return                          Whether this `Result` holds a value.
     */
    [[nodiscard]] bool ok() const {
        return _impl.has_value();
    }

    [[nodiscard]] explicit operator bool() const {
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
    std::expected<T, Error> _impl;
};

template<>
class [[nodiscard]] Result<void> {
 public:
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
 * Same as the above, but for an error that carries an `std::error_code`.
 *
 * @param code                          Error code to attach.
 * @param fmt                           Format string.
 * @param args                          Format arguments.
 * @return                              `Error` with the formatted message and the error code.
 */
template<class... Args>
[[nodiscard]] Error fail(std::error_code code, fmt::format_string<Args...> fmt, Args &&... args) {
    return Error(code, fmt, std::forward<Args>(args)...);
}

/**
 * Explicitly discards a `Result`, error and all.
 *
 * `Result` is `[[nodiscard]]`, so simply ignoring a return value doesn't compile – we don't want errors to be
 * dropped by accident. This function is the way to drop one *on purpose*, and it's greppable:
 * ```
 * discard(ufs->remove(path)); // We don't care whether it existed.
 * ```
 *
 * @param result                        Result to discard.
 */
template<class T>
void discard(Result<T> &&result) {} // NOLINT(misc-unused-parameters)

/**
 * Runs a callable, converting any exception it throws into an `Error`.
 *
 * This is the bridge from code that throws – third-party libraries (nlohmann/json, sol2, CLI11, the standard
 * library), and the parts of our own code that haven't been converted to `Result` yet. Exceptions should not travel
 * any further up the stack than this.
 *
 * The callable itself may also return a `Result` – it won't get double-wrapped, so it's OK to mix `fail()` /
 * `MM_TRY` with throwing calls inside one `tryCatch` block.
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

#define MM_DETAIL_TRY_CONCAT_2(a, b) a##b
#define MM_DETAIL_TRY_CONCAT(a, b) MM_DETAIL_TRY_CONCAT_2(a, b)
#define MM_DETAIL_TRY_TMP MM_DETAIL_TRY_CONCAT(_mmTryTmp, __COUNTER__)

#define MM_DETAIL_TRY(tmp, decl, ...)                                                                                  \
    auto tmp = (__VA_ARGS__);                                                                                          \
    if (!tmp) /* NOLINT */                                                                                             \
        return std::move(tmp).error();                                                                                 \
    decl = *std::move(tmp)

#define MM_DETAIL_TRY_VOID(tmp, ...)                                                                                   \
    do {                                                                                                               \
        auto tmp = (__VA_ARGS__);                                                                                      \
        if (!tmp) /* NOLINT */                                                                                         \
            return std::move(tmp).error();                                                                             \
    } while (false)

/**
 * Evaluates a `Result`-returning expression, and either unpacks its value into `decl`, or early-returns the error
 * from the enclosing function. The enclosing function must itself return a `Result`.
 *
 * ```
 * MM_TRY(Blob data, fs->read(path));   // Declares `Blob data`.
 * MM_TRY(auto entries, reader.ls());   // `auto` works, too.
 * MM_TRY(_pixels, decodePixels(data)); // So does assigning into an existing variable.
 * ```
 *
 * Note that `decl` must not contain a top-level comma – use `auto` if you need to declare something like an
 * `std::pair<int, int>`.
 *
 * Also note that this macro expands into several statements, and so needs braces when used as a body of an `if` or
 * a loop. Getting this wrong is a compilation error, not a silent bug. `MM_TRY_VOID` doesn't have this problem.
 *
 * @param decl                          Declaration (or an lvalue) to unpack the value into.
 * @param ...                           Expression evaluating to a `Result`.
 */
#define MM_TRY(decl, ...) MM_DETAIL_TRY(MM_DETAIL_TRY_TMP, decl, __VA_ARGS__)

/**
 * Same as `MM_TRY`, but discards the value. Use it for `Result<void>`, or when the value isn't needed.
 *
 * @param ...                          Expression evaluating to a `Result`.
 */
#define MM_TRY_VOID(...) MM_DETAIL_TRY_VOID(MM_DETAIL_TRY_TMP, __VA_ARGS__)
