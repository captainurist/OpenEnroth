#pragma once

#include <deque>
#include <string>
#include <vector>

#include "Utility/Error/Result.h"

/**
 * Utility class that turns on UTF-8 for most of CRT, and converts command-line arguments to UTF-8. This is really only
 * needed on Windows, and this class does nothing on POSIX.
 *
 * Use it like this:
 * ```
 * int main(int argc, char **argv) {
 *     UnicodeCrt crt = UnicodeCrt::create(argc, argv).orThrow(); // Or .mustSucceed() where there's no catch above.
 *     // Use argc & argv here, and keep `crt` alive while doing so.
 * }
 * ```
 *
 * Note that for this to work on older Windows versions, CRT should be statically linked. This is how OE releases
 * are built right now.
 *
 *
 * @see https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/setlocale-wsetlocale?view=msvc-170#utf-8-support
 */
class UnicodeCrt {
 public:
    /**
     * @param[in,out] argc              Argument count, as passed into `main`.
     * @param[in,out] argv              Argument values, as passed into `main`. Replaced with UTF-8 encoded
     *                                  arguments that stay valid for as long as the returned object is alive.
     * @return                          An object that keeps the converted arguments alive, or an error if the CRT
     *                                  couldn't be switched to UTF-8. This shouldn't normally fail, but it can in
     *                                  theory.
     */
    static Result<UnicodeCrt> create(int &argc, char **&argv);

    /**
     * @return                          Whether a `UnicodeCrt` was created, and thus CRT now uses UTF-8.
     */
    static bool isInitialized();

 private:
    UnicodeCrt() = default;

 private:
    std::vector<char *> _argv;
    std::deque<std::string> _storage; // Deque, so that moving `UnicodeCrt` around doesn't invalidate the pointers
                                      // in `_argv` - deques never relocate their elements.
};
