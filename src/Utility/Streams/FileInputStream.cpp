#include "FileInputStream.h"

#include <cassert>
#include <algorithm>
#include <memory>
#include <string>
#include <filesystem>

#include "Utility/Exception.h"
#include "Utility/UnicodeCrt.h"

FileInputStream::FileInputStream(std::string_view path, size_t bufferSize) {
    open(path, bufferSize);
}

FileInputStream::~FileInputStream() {
    destroy();
}

void FileInputStream::open(std::string_view path, size_t bufferSize) {
    // `std::filesystem` on Windows converts between narrow and wide strings using the CRT locale, so the CRT does have
    // to be in UTF-8 mode here. Note that the file opening itself doesn't depend on this, as it goes through
    // `CreateFileW`.
    //
    // TODO(captainurist): this assert can go away by transcoding explicitly instead of relying on the CRT locale -
    // `wideToUtf8(absolute(std::filesystem::path(utf8ToWide(path))).generic_wstring())` on Windows. Note that
    // `Blob::fromFile` and `DirectoryFileSystem` have to move over at the same time, as
    // `DirectoryFileSystem.DisplayPathSymmetry` requires our display path to match `Blob`'s byte-for-byte.
    assert(UnicodeCrt::isInitialized());
    assert(bufferSize > 0);
    assert(!isOpen()); // Reopening an open stream is a bug, close it first.

    std::string absolutePath = absolute(std::filesystem::path(path)).generic_string();

    _handle.openForReading(absolutePath); // Throws on error, leaving this object untouched.
    _buf.reset(); // Might have been allocated for a different buffer size.
    _bufSize = bufferSize;
    base_type::open({}, _handle.size(), absolutePath);
}

size_t FileInputStream::_underflow(void *data, size_t size, Buffer *buffer) {
    assert(buffer->remaining() == 0);

    // Note that in refill mode (`size == 0`, called from `readUntilSlow`) `position()` over-reports by
    // `buffer->used()`, and thus must not be used here. This is why the code below goes through `_handle`.
    //
    // Note that this also holds after a failed read - `FileHandle` only throws from operations that didn't move the
    // file offset, reporting a partial transfer as a short read instead.
    assert(size == 0 || _handle.offset() == position());

    if (size < _bufSize) {
        // Small read/skip/refill: fill the internal buffer.
        if (!_buf)
            _buf = std::make_unique_for_overwrite<char[]>(_bufSize);
        size_t bytesRead = _handle.read(_buf.get(), _bufSize);

        // Note that `used()` has to be zero here, that's what the refill mode requires.
        buffer->reset(_buf.get(), _buf.get(), _buf.get() + bytesRead);
        if (data) {
            return buffer->read(data, std::min(size, bytesRead));
        } else {
            return buffer->skip(std::min(size, bytesRead));
        }
    } else if (data) {
        // Large read: read directly into the target buffer, leaving our buffer alone.
        return _handle.read(data, size);
    } else {
        // Large skip: seek. Clamping is needed because seeking past the end of a file succeeds on POSIX.
        size_t bytesToSkip = std::min(size, _handle.bytesLeft());
        _handle.seekForward(bytesToSkip);
        return bytesToSkip;
    }
}

void FileInputStream::_close(bool canThrow) {
    assert(isOpen());

    std::string path = displayPath(); // `base_type::_close` below clears it.
    int error = _handle.close();
    _buf.reset();
    _bufSize = 0;

    // Note that the base class is notified even if closing has failed - this stream no longer holds a file handle,
    // and `isOpen()` must not keep returning `true`.
    base_type::_close(canThrow);

    if (error != 0 && canThrow) // TODO(captainurist): !canThrow => log OR attach
        Exception::throwFromOsError(error, path);
}
