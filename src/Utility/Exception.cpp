#include "Exception.h"

#ifdef _WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#endif

#include <cassert>
#include <cerrno>
#include <system_error>

int Exception::lastOsError() {
#ifdef _WINDOWS
    return static_cast<int>(GetLastError());
#else
    return errno;
#endif
}

void Exception::throwFromOsError(std::string_view arg) {
    throwFromOsError(lastOsError(), arg);
}

void Exception::throwFromOsError(int error, std::string_view arg) {
    assert(error != 0);

    // Note that the error category differs between platforms. `std::system_category` is the native OS error category,
    // which means Win32 error codes on Windows, while `errno` values belong to `std::generic_category`. On POSIX the
    // two categories are equivalent, but on Windows passing an `errno` value into `std::system_category` produces
    // a message for a completely unrelated Win32 error.
#ifdef _WINDOWS
    throw Exception("{}: {}", arg, std::system_category().message(error));
#else
    throw Exception("{}: {}", arg, std::generic_category().message(error));
#endif
}

void Exception::throwFromErrc(std::errc error, std::string_view arg) {
    assert(error != std::errc());

    throw Exception("{}: {}", arg, std::make_error_code(error).message());
}
