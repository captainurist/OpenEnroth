#include "FileOutputStream.h"

#include <cassert>
#include <memory>
#include <string>
#include <filesystem>

#include "Utility/Exception.h"
#include "Utility/ScopeGuard.h"
#include "Utility/UnicodeCrt.h"

FileOutputStream::FileOutputStream(std::string_view path, size_t bufferSize) {
    open(path, bufferSize);
}

FileOutputStream::~FileOutputStream() {
    destroy();
}

void FileOutputStream::open(std::string_view path, size_t bufferSize) {
    // TODO(captainurist): this assert can go away, see the comment in `FileInputStream::open`.
    assert(UnicodeCrt::isInitialized());
    assert(bufferSize > 0);
    assert(!isOpen()); // Reopening an open stream is a bug, close it first.

    std::string absolutePath = absolute(std::filesystem::path(path)).generic_string();

    _handle.openForWriting(absolutePath); // Throws on error, leaving this object untouched.
    _buf.reset(); // Might have been allocated for a different buffer size.
    _bufSize = bufferSize;
    _failed = false;
    base_type::open({}, absolutePath);
}

void FileOutputStream::_overflow(Buffer *buffer, const void *data, size_t size) {
    if (size < _bufSize) {
        // Small write: fill current buffer, write it all out, put the tail into a fresh buffer.
        size_t head = buffer->write(data, buffer->remaining());
        writeBuffer(buffer, true);
        data = static_cast<const char *>(data) + head;
        size -= head;
        if (!_buf)
            _buf = std::make_unique_for_overwrite<char[]>(_bufSize);
        buffer->reset(_buf.get(), _buf.get(), _buf.get() + _bufSize);
        buffer->write(data, size);
    } else {
        // Large write: write out current buffer, then write data directly.
        writeBuffer(buffer, true);
        _handle.write(data, size);
        if (_buf) // If `_buf` is null then `buffer` is the shared empty buffer, and must not be touched.
            buffer->reset(_buf.get(), _buf.get(), _buf.get() + _bufSize);
    }
}

void FileOutputStream::_flush(Buffer *buffer) {
    // Note that we deliberately don't `fsync` here - handing the data over to the OS is all that's needed, and
    // `StreamLogSink` flushes after every single log line.
    writeBuffer(buffer, true);
}

void FileOutputStream::_close(Buffer *buffer, bool canThrow) {
    assert(isOpen());

    std::string path = displayPath(); // `base_type::_close` below clears it.
    int closeError = 0;

    {
        // Whatever happens below, this stream ends up closed. Releasing the handle and notifying the base class on all
        // paths is what makes sure that a failed write is not retried into an already closed stream by the destructor.
        MM_AT_SCOPE_EXIT(
            closeError = _handle.close();
            _buf.reset();
            _bufSize = 0;
            base_type::_close(buffer, canThrow));

        writeBuffer(buffer, canThrow);
    }

    if (closeError != 0 && canThrow) // TODO(captainurist): !canThrow => log OR attach
        Exception::throwFromOsError(closeError, path);
}

void FileOutputStream::writeBuffer(Buffer *buffer, bool canThrow) {
    size_t size = buffer->used();
    if (size == 0 || _failed)
        return;

    try {
        _handle.write(buffer->start(), size);
    } catch (const std::exception &) { // Note that this also catches `std::bad_alloc` from building the error message,
                                       // which must not escape when `canThrow` is false - we're inside a destructor.
        // A partial write might have already made it out, so retrying this buffer from `_close` would duplicate its
        // prefix. Give up on this stream instead.
        //
        // Note that the buffer is deliberately NOT committed here. `commit()` drops `used()`, and the base class only
        // folds that amount into `_bufferBase` after `_flush` / `_overflow` return normally - so committing on the
        // throwing path would make `position()` jump backwards by up to a full buffer.
        _failed = true;
        if (canThrow)
            throw;
        return; // TODO(captainurist): !canThrow => log OR attach
    }

    buffer->commit();
}
