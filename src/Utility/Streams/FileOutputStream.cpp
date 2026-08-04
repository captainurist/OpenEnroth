#include "FileOutputStream.h"

#include <cassert>
#include <cstdio>
#include <memory>
#include <string>
#include <filesystem>

#include "Utility/UnicodeCrt.h"

FileOutputStream::~FileOutputStream() {
    destroy();
}

Result<void> FileOutputStream::open(std::string_view path, size_t bufferSize) {
    assert(UnicodeCrt::isInitialized()); // Otherwise fopen on Windows will choke on UTF-8 paths.
    assert(bufferSize > 0);

    std::string absPath = absolute(std::filesystem::path(path)).generic_string();
    _file = fopen(absPath.c_str(), "wb");
    if (!_file)
        return Error::fromErrno(absPath);

    // Disable libc buffering, we manage our own buffer.
    if (setvbuf(_file, nullptr, _IONBF, 0) != 0)
        return Error::fromErrno(absPath);

    _bufSize = bufferSize;
    base_type::open({}, absPath);
    return {};
}

Result<void> FileOutputStream::_overflow(Buffer *buffer, const void *data, size_t size) {
    if (size < _bufSize) {
        // Small write: fill current buffer, write it all out, put the tail into a fresh buffer.
        size_t head = buffer->write(data, buffer->remaining());
        if (Result<void> result = writeBuffer(*buffer); !result) [[unlikely]]
            return result;
        data = static_cast<const char *>(data) + head;
        size -= head;
        if (!_buf)
            _buf = std::make_unique<char[]>(_bufSize);
        buffer->reset(_buf.get(), _buf.get(), _buf.get() + _bufSize);
        buffer->write(data, size);
        return {};
    } else {
        // Large write: write out current buffer, then write data directly.
        if (Result<void> result = writeBuffer(*buffer); !result) [[unlikely]]
            return result;
        if (fwrite(data, size, 1, _file) != 1)
            return Error::fromErrno(displayPath());
        if (_buf)
            buffer->reset(_buf.get(), _buf.get(), _buf.get() + _bufSize);
        return {};
    }
}

Result<void> FileOutputStream::_flush(Buffer *buffer) {
    if (Result<void> result = writeBuffer(*buffer); !result) [[unlikely]]
        return result;
    buffer->commit();
    if (fflush(_file) != 0)
        return Error::fromErrno(displayPath());
    return {};
}

Result<void> FileOutputStream::_close(Buffer *buffer) {
    assert(isOpen());

    Result<void> writeResult = writeBuffer(*buffer);

    int status = fclose(_file);
    _file = nullptr;
    _buf.reset();
    _bufSize = 0;

    Result<void> baseResult = base_type::_close(buffer);
    if (!writeResult)
        return writeResult;
    if (status != 0)
        return Error::fromErrno(displayPath());
    return baseResult;
}

Result<void> FileOutputStream::writeBuffer(const Buffer &buffer) {
    if (size_t bytesBuffered = buffer.used())
        if (fwrite(buffer.start(), bytesBuffered, 1, _file) != 1)
            return Error::fromErrno(displayPath());
    return {};
}
