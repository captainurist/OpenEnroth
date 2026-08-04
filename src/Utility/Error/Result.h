#pragma once

#include <expected>
#include <type_traits>
#include <utility>

#include "Utility/Exception.h"

#include "Error.h"

/**
 * Return type for functions that can fail in a recoverable way.
 *
 * `Result<T>` is an `std::expected<T, Error>` – it holds either a `T`, or an `Error` explaining why the `T` couldn't
 * be produced. It's the replacement for throwing an `Exception`: instead of unwinding the stack from somewhere deep
 * inside the engine, a function makes the failure part of its signature, and the caller is forced by
 * `[[nodiscard]]` to decide what to do about it.
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
 *     MM_TRY(Blob data, fs->read(name));                              // Declares `Blob data`.
 *     MM_TRY(Font font, withContext(oef::decode(data), "while loading font '{}'", name));
 *     MM_TRY_VOID(validate(font));                                    // Value-less call.
 *     return font;
 * }
 * ```
 *
 * Handling an error – pick one of the three policies, explicitly:
 * ```
 * // 1. Propagate, adding context. See MM_TRY above.
 *
 * // 2. Degrade. The game keeps running with a fallback.
 * Result<RgbaImage> image = pcx::decode(thumbnail);
 * if (!image) {
 *     logger->warning("Bad savegame thumbnail: {}", image.error());
 *     return nullptr;
 * }
 *
 * // 3. Die, loudly and on purpose. Only for things that make the game unusable.
 * Blob icon = mustSucceed(dfs->read("images/OpenEnroth.png"));
 * ```
 *
 * @see Error
 */
template<class T = void>
using Result = std::expected<T, Error>;

/**
 * Creates a failed `Result`.
 *
 * The return type is implicitly convertible to `Result<T>` for any `T`, so this is meant to be used directly in a
 * `return` statement:
 * ```
 * return fail("Entry '{}' doesn't exist in LOD file '{}'", filename, _lod.displayPath());
 * ```
 *
 * @param fmt                           Format string.
 * @param args                          Format arguments.
 * @return                              Error wrapped into an `std::unexpected`.
 */
template<class... Args>
[[nodiscard]] std::unexpected<Error> fail(fmt::format_string<Args...> fmt, Args &&... args) {
    return std::unexpected(Error(fmt, std::forward<Args>(args)...));
}

/**
 * Same as the above, but for an error that carries an `std::error_code`.
 *
 * @param code                          Error code to attach.
 * @param fmt                           Format string.
 * @param args                          Format arguments.
 * @return                              Error wrapped into an `std::unexpected`.
 */
template<class... Args>
[[nodiscard]] std::unexpected<Error> fail(std::error_code code, fmt::format_string<Args...> fmt, Args &&... args) {
    return std::unexpected(Error(code, fmt, std::forward<Args>(args)...));
}

/**
 * Adds a context frame to a failed `Result`, passing successful ones through unchanged.
 *
 * @param result                        Result to add context to.
 * @param fmt                           Format string describing what the caller was doing.
 * @param args                          Format arguments.
 * @return                              `result`, with an additional context frame if it holds an error.
 */
template<class T, class... Args>
[[nodiscard]] Result<T> withContext(Result<T> result, fmt::format_string<Args...> fmt, Args &&... args) {
    if (result)
        return result;
    return std::unexpected(result.error().withContext(fmt, std::forward<Args>(args)...));
}

/**
 * Unwraps a `Result`, terminating the process through `fatalError` if it holds an error.
 *
 * This is the explicit, greppable equivalent of letting an exception escape into `main`. Use it only where there is
 * genuinely nothing to fall back to – e.g. when the game data folder turns out to be unreadable during startup.
 * Never use it on anything that runs inside the game loop.
 *
 * @param result                        Result to unwrap.
 * @return                              The value held by `result`.
 */
template<class T>
T mustSucceed(Result<T> &&result) {
    if (!result)
        fatalError(result.error());
    return *std::move(result);
}

template<class T>
T mustSucceed(const Result<T> &result) {
    if (!result)
        fatalError(result.error());
    return *result;
}

/**
 * Unwraps a `Result`, throwing an `Exception` if it holds an error.
 *
 * This is for code that legitimately wants to handle errors by throwing:
 * - Command-line tools and unit tests, where the top-level `catch` is the error handling, and there is no game loop
 *   to keep running.
 * - Engine code that hasn't been converted to `Result` yet – there, every use of this function is a `TODO`.
 *
 * Never use it in the game itself. `mustSucceed` is the honest way to say "this can't fail" there.
 *
 * @param result                        Result to unwrap.
 * @return                              The value held by `result`.
 * @throws Exception                    If `result` holds an error.
 */
template<class T>
T orThrow(Result<T> result) {
    if (!result)
        throw Exception("{}", result.error().message());
    return *std::move(result);
}

/**
 * Runs a callable, converting any exception it throws into an `Error`.
 *
 * This is the bridge to third-party code that we can't make exception-free – e.g. the JSON library, the Lua
 * bindings, or the standard library itself. Exceptions should not travel any further up the stack than this.
 *
 * @param callable                      Callable to run.
 * @return                              Whatever `callable` returned, or the exception it threw, as an `Error`.
 */
template<class Callable>
[[nodiscard]] auto tryCatch(Callable &&callable) -> Result<std::invoke_result_t<Callable>> {
    try {
        if constexpr (std::is_void_v<std::invoke_result_t<Callable>>) {
            std::forward<Callable>(callable)();
            return {};
        } else {
            return std::forward<Callable>(callable)();
        }
    } catch (...) {
        return std::unexpected(Error::fromCurrentException());
    }
}

#define MM_DETAIL_TRY_CONCAT_2(a, b) a##b
#define MM_DETAIL_TRY_CONCAT(a, b) MM_DETAIL_TRY_CONCAT_2(a, b)
#define MM_DETAIL_TRY_TMP MM_DETAIL_TRY_CONCAT(_mmTryTmp, __COUNTER__)

#define MM_DETAIL_TRY(tmp, decl, ...)                                                                                  \
    auto tmp = (__VA_ARGS__);                                                                                          \
    if (!tmp) /* NOLINT */                                                                                             \
        return std::unexpected(std::move(tmp).error());                                                                \
    decl = *std::move(tmp)

#define MM_DETAIL_TRY_VOID(tmp, ...)                                                                                   \
    do {                                                                                                               \
        auto tmp = (__VA_ARGS__);                                                                                      \
        if (!tmp) /* NOLINT */                                                                                         \
            return std::unexpected(std::move(tmp).error());                                                            \
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
 * @param ...                           Expression evaluating to a `Result`.
 */
#define MM_TRY_VOID(...) MM_DETAIL_TRY_VOID(MM_DETAIL_TRY_TMP, __VA_ARGS__)
