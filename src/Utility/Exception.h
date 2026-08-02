#pragma once

#include <stdexcept>
#include <string_view>
#include <utility>

#include "Utility/String/Format.h"

class Exception : public std::runtime_error {
 public:
    template<class... Args>
    explicit Exception(fmt::format_string<Args...> fmt, Args&&... args) : std::runtime_error(fmt::format(fmt, std::forward<Args>(args)...)) {}

    /**
     * @return                          Code of the last OS error - `errno` on POSIX, `GetLastError()` on Windows.
     */
    [[nodiscard]] static int lastOsError();

    /**
     * Throws an `Exception` describing the last OS error, see `lastOsError()`.
     *
     * @param arg                       Context to prepend to the error message, usually a file path.
     * @throws Exception                Always.
     */
    [[noreturn]] static void throwFromOsError(std::string_view arg);

    /**
     * Same as the above, but for an error code that was retrieved earlier via `lastOsError()`. Needed when the failing
     * call has to be followed by cleanup that would clobber `errno` / `GetLastError()`.
     *
     * @param error                     OS error code.
     * @param arg                       Context to prepend to the error message, usually a file path.
     * @throws Exception                Always.
     */
    [[noreturn]] static void throwFromOsError(int error, std::string_view arg);

    [[noreturn]] static void throwFromErrc(std::errc error, std::string_view arg);
};
