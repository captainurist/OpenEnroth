#include "FileHandle.h"

#ifdef _WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#else
#   include <fcntl.h>
#   include <sys/stat.h>
#   include <unistd.h>
#endif

#include <cassert>
#include <cerrno>
#include <algorithm>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>

#include "Utility/Exception.h"
#include "Utility/ScopeGuard.h"

#ifdef _WINDOWS
#   include "Utility/String/Encoding.h"

static_assert(std::is_same_v<FileHandle::NativeHandle, HANDLE>);
#endif

namespace {

// Largest chunk we pass to a single OS-level read / write. `ReadFile` / `WriteFile` on Windows take a `DWORD`, and
// Linux caps a single `read` / `write` at slightly under 2Gb.
constexpr size_t MAX_IO_CHUNK_SIZE = 0x7ffff000;

/**
 * Closes a native handle, ignoring errors. Used on error paths where we already have an exception to throw.
 */
void closeNativeHandle(FileHandle::NativeHandle handle) {
#ifdef _WINDOWS
    CloseHandle(handle);
#else
    ::close(handle);
#endif
}

} // namespace

FileHandle::~FileHandle() {
    (void) close();
}

void FileHandle::openForReading(std::string_view path) {
    open(path, false);
}

void FileHandle::openForWriting(std::string_view path) {
    open(path, true);
}

void FileHandle::open(std::string_view path, bool forWriting) {
    // Reopening an already open handle is a caller bug, but closing here means that we never leak in release builds.
    assert(!isOpen());
    (void) close();

    std::string displayPath(path);
    NativeHandle handle;

#ifdef _WINDOWS
    // Note the share mode - `fopen` on Windows opens files with `_SH_DENYNO`, which is exactly
    // `FILE_SHARE_READ | FILE_SHARE_WRITE`, and we have to match that, otherwise opening a file for reading while
    // it's still open for writing would fail. `FILE_SHARE_DELETE` is deliberately not in there - `fopen` doesn't
    // pass it either, and we don't want a file to be deletable while we're reading it.
    //
    // Also note that we're not passing `FILE_FLAG_BACKUP_SEMANTICS`, and thus opening a directory fails here, which
    // is what we want.
    handle = CreateFileW(txt::utf8ToWide(displayPath).c_str(),
                         forWriting ? GENERIC_WRITE : GENERIC_READ,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         nullptr,
                         forWriting ? CREATE_ALWAYS : OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         nullptr);

    // Note that `CREATE_ALWAYS` sets the last error to `ERROR_ALREADY_EXISTS` even on success, so the returned handle
    // is the only thing that can be checked here.
    if (handle == INVALID_HANDLE_VALUE)
        Exception::throwFromOsError(displayPath);
#else
    int flags = forWriting ? (O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC) : (O_RDONLY | O_CLOEXEC);
    do {
        handle = ::open(displayPath.c_str(), flags, 0666); // Same as what `fopen("wb")` does, modulo umask.
    } while (handle < 0 && errno == EINTR);
    if (handle < 0)
        Exception::throwFromOsError(displayPath);
#endif

    // The handle is ours from this point on, so make sure it's released if anything below throws.
    bool succeeded = false;
    MM_AT_SCOPE_EXIT(if (!succeeded) closeNativeHandle(handle));

    if (!forWriting) {
#ifdef _WINDOWS
        // `CreateFileW` already refuses directories, but pipes and consoles do open, and `size()` makes no sense for
        // them. Reject here so that we behave the same way on all platforms.
        if (GetFileType(handle) != FILE_TYPE_DISK)
            throw Exception("{}: not a regular file", displayPath);
#else
        struct stat fileStat = {};
        if (fstat(handle, &fileStat) != 0)
            Exception::throwFromOsError(displayPath);

        // `open` happily opens directories for reading, but reading from them always fails. Fail early instead, so
        // that we behave the same way on all platforms - `CreateFileW` just fails for directories.
        if (S_ISDIR(fileStat.st_mode))
            Exception::throwFromErrc(std::errc::is_a_directory, displayPath);

        // Same for pipes and character devices - `st_size` is meaningless for those, so `size()` would lie.
        if (!S_ISREG(fileStat.st_mode))
            throw Exception("{}: not a regular file", displayPath);
#endif
    }

    succeeded = true;
    _handle = handle;
    _displayPath = std::move(displayPath);
}

int FileHandle::close() {
    if (!isOpen())
        return 0;

    // Note that the handle is released before the result is examined. On Linux the file descriptor is always released,
    // even when `close` returns `EINTR`, and retrying the call is a file descriptor reuse race.
    NativeHandle handle = std::exchange(_handle, INVALID_HANDLE);
#ifdef _WINDOWS
    bool succeeded = CloseHandle(handle) != 0;
#else
    bool succeeded = ::close(handle) == 0;
#endif
    int error = succeeded ? 0 : Exception::lastOsError();

    _displayPath = {};
    return error;
}

size_t FileHandle::read(void *data, size_t size) {
    assert(isOpen());
    assert(data || size == 0);

    char *dst = static_cast<char *>(data);
    size_t bytesRead = 0;
    while (bytesRead < size) {
        size_t chunkSize = std::min(size - bytesRead, MAX_IO_CHUNK_SIZE);

        // Note that a failure is only reported if nothing has been read yet. Reporting a short read instead keeps our
        // file offset in sync with what the caller thinks it has consumed - `InputStream` only updates its accounting
        // once `_underflow` returns normally, so throwing here after having already moved the offset would desync the
        // two for good. The caller retries, and then gets the error from a read that consumed nothing.
#ifdef _WINDOWS
        DWORD chunkRead = 0;
        if (!ReadFile(_handle, dst + bytesRead, static_cast<DWORD>(chunkSize), &chunkRead, nullptr)) {
            if (bytesRead > 0)
                break;
            Exception::throwFromOsError(_displayPath);
        }
#else
        ssize_t chunkRead = ::read(_handle, dst + bytesRead, chunkSize);
        if (chunkRead < 0) {
            if (errno == EINTR)
                continue; // Interrupted before reading anything, just retry.
            if (bytesRead > 0)
                break;
            Exception::throwFromOsError(_displayPath);
        }
#endif

        if (chunkRead == 0)
            break; // End of file.

        bytesRead += chunkRead;
    }

    return bytesRead;
}

void FileHandle::write(const void *data, size_t size) {
    assert(isOpen());
    assert(data || size == 0);

    const char *src = static_cast<const char *>(data);
    size_t bytesWritten = 0;
    while (bytesWritten < size) {
        size_t chunkSize = std::min(size - bytesWritten, MAX_IO_CHUNK_SIZE);

#ifdef _WINDOWS
        DWORD chunkWritten = 0;
        if (!WriteFile(_handle, src + bytesWritten, static_cast<DWORD>(chunkSize), &chunkWritten, nullptr))
            Exception::throwFromOsError(_displayPath);
#else
        ssize_t chunkWritten = ::write(_handle, src + bytesWritten, chunkSize);
        if (chunkWritten < 0) {
            if (errno == EINTR)
                continue;
            Exception::throwFromOsError(_displayPath);
        }
#endif

        // Shouldn't happen for regular files, but we don't want to spin here forever if it does.
        if (chunkWritten == 0)
            throw Exception("{}: write didn't make any progress", _displayPath);

        bytesWritten += chunkWritten;
    }
}

size_t FileHandle::size() const {
    assert(isOpen());

    uint64_t result;
#ifdef _WINDOWS
    LARGE_INTEGER nativeSize;
    if (!GetFileSizeEx(_handle, &nativeSize))
        Exception::throwFromOsError(_displayPath);
    result = static_cast<uint64_t>(nativeSize.QuadPart);

    // `InputStream` uses `size_t` for stream positions, so there's no way for us to handle files larger than 4Gb on
    // 32-bit builds. This can't trigger on POSIX, where `st_size` is an `off_t` that's never wider than `size_t`.
    if (result > std::numeric_limits<size_t>::max())
        throw Exception("{}: file is too large, {} bytes", _displayPath, result);
#else
    struct stat fileStat = {};
    if (fstat(_handle, &fileStat) != 0)
        Exception::throwFromOsError(_displayPath);
    result = static_cast<uint64_t>(fileStat.st_size);
#endif

    return static_cast<size_t>(result);
}

void FileHandle::seek(size_t position) {
    assert(isOpen());

#ifdef _WINDOWS
    LARGE_INTEGER distance;
    distance.QuadPart = static_cast<LONGLONG>(position);
    if (!SetFilePointerEx(_handle, distance, nullptr, FILE_BEGIN))
        Exception::throwFromOsError(_displayPath);
#else
    // Callers only ever seek inside the file, and file sizes come from `st_size`, which is an `off_t`.
    assert(position <= static_cast<size_t>(std::numeric_limits<off_t>::max()));
    if (lseek(_handle, static_cast<off_t>(position), SEEK_SET) == static_cast<off_t>(-1))
        Exception::throwFromOsError(_displayPath);
#endif
}
