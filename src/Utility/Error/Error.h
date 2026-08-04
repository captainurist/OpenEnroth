#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include "Utility/String/Format.h"

/**
 * A recoverable error.
 *
 * `Error` is what an `Error`-returning function passes back to its caller instead of throwing. It carries a
 * human-readable message, an optional `std::error_code` for programmatic handling, and an optional chain of
 * context frames that describe what the code was doing when the error happened.
 *
 * Constructing an `Error` is the same as constructing an `Exception` – pass a format string and its arguments:
 * ```
 * Error("Cannot decode PCX image '{}': invalid version {}", data.displayPath(), version)
 * ```
 *
 * Context frames are added as the error propagates up the stack, so that the innermost code doesn't have to know
 * (or repeat) the outer context:
 * ```
 * Error inner("unexpected end of stream");
 * Error outer = inner.withContext("cannot read SND header of '{}'", path);
 * outer.message(); // "cannot read SND header of 'sounds.snd': unexpected end of stream"
 * ```
 *
 * `Error` is cheap to copy and move – it's a single `shared_ptr` under the hood, so `sizeof(Result<T>)` stays close
 * to `sizeof(T)`. All the expensive work (formatting, allocating) happens only on the error path.
 *
 * @see Result
 */
class Error {
 public:
    /**
     * Constructs an error with a formatted message.
     *
     * @param fmt                       Format string.
     * @param args                      Format arguments.
     */
    template<class... Args>
    explicit Error(fmt::format_string<Args...> fmt, Args &&... args) :
        _node(std::make_shared<const Node>(::fmt::format(fmt, std::forward<Args>(args)...), std::error_code(), nullptr)) {}

    /**
     * Constructs an error carrying an error code, with a formatted message.
     *
     * The message of the error code itself is not included in the message – use `withContext` or format it in
     * explicitly if you need it.
     *
     * @param code                      Error code to attach.
     * @param fmt                       Format string.
     * @param args                      Format arguments.
     */
    template<class... Args>
    Error(std::error_code code, fmt::format_string<Args...> fmt, Args &&... args) :
        _node(std::make_shared<const Node>(::fmt::format(fmt, std::forward<Args>(args)...), code, nullptr)) {}

    /**
     * @param arg                       Description of the operation that failed, e.g. a file path.
     * @return                          Error created from the current value of `errno`, with the message describing
     *                                  the error code appended to `arg`.
     */
    [[nodiscard]] static Error fromErrno(std::string_view arg);

    /**
     * @param error                     Error code.
     * @param arg                       Description of the operation that failed, e.g. a file path.
     * @return                          Error created from the given error code, with the message describing the error
     *                                  code appended to `arg`.
     */
    [[nodiscard]] static Error fromErrc(std::errc error, std::string_view arg);

    /**
     * Creates an `Error` from the exception that's currently being handled. Intended to be used at the boundaries
     * with third-party code that throws (e.g. Lua bindings, or the JSON library).
     *
     * Must be called from inside a `catch` block.
     *
     * @return                          Error describing the exception that's currently being handled.
     */
    [[nodiscard]] static Error fromCurrentException();

    /**
     * @param fmt                       Format string describing what the caller was doing.
     * @param args                      Format arguments.
     * @return                          A copy of this error with an additional context frame prepended.
     */
    template<class... Args>
    [[nodiscard]] Error withContext(fmt::format_string<Args...> fmt, Args &&... args) const {
        return Error(std::make_shared<const Node>(::fmt::format(fmt, std::forward<Args>(args)...), std::error_code(), _node));
    }

    /**
     * @return                          Full error message, with all context frames joined with `": "`, outermost
     *                                  first.
     */
    [[nodiscard]] std::string message() const;

    /**
     * @return                          Message of the outermost context frame only.
     */
    [[nodiscard]] std::string_view outerMessage() const {
        return _node->message;
    }

    /**
     * @return                          Message of the innermost frame – the one describing the error that actually
     *                                  happened, without any of the context that was added on the way up.
     */
    [[nodiscard]] std::string_view innerMessage() const;

    /**
     * @return                          Error code of the innermost frame that has one, or a default-constructed
     *                                  `std::error_code` if no frame carries one.
     */
    [[nodiscard]] std::error_code code() const;

 private:
    struct Node {
        std::string message;
        std::error_code code;
        std::shared_ptr<const Node> cause;
    };

    explicit Error(std::shared_ptr<const Node> node) : _node(std::move(node)) {}

 private:
    std::shared_ptr<const Node> _node;
};

/**
 * Handler that's invoked when an unrecoverable error happens, e.g. from `mustSucceed`.
 *
 * Installing a handler lets the game show the error to the user (e.g. in a message box) before going down. The
 * handler is expected not to return – if it does, `std::abort` is called.
 */
using FatalErrorHandler = void (*)(const Error &);

/**
 * Installs a global fatal error handler.
 *
 * @param handler                       Handler to install, or `nullptr` to reset to the default one, which just
 *                                      prints the error to `stderr`.
 * @return                              Previously installed handler.
 */
FatalErrorHandler setFatalErrorHandler(FatalErrorHandler handler);

/**
 * Reports an unrecoverable error and terminates the process. Never returns.
 *
 * @param error                         Error to report.
 */
[[noreturn]] void fatalError(const Error &error);

template<>
struct fmt::formatter<Error> : fmt::formatter<std::string> {
    auto format(const Error &error, fmt::format_context &ctx) const {
        return fmt::formatter<std::string>::format(error.message(), ctx);
    }
};
