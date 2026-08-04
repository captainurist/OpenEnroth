#include "FileInputStream.h"

#include <cassert>
#include <cstdio>
#include <algorithm>
#include <memory>
#include <string>
#include <filesystem>

#include "Utility/UnicodeCrt.h"

#ifdef _WINDOWS
#   define ftello _ftelli64
#   define fseeko _fseeki64
#endif

FileInputStream::~FileInputStream() {
    destroy();
}

Result<void> FileInputStream::open(std::string_view path, size_t bufferSize) {
    assert(UnicodeCrt::isInitialized()); // Otherwise fopen on Windows will choke on UTF-8 paths.
    assert(bufferSize > 0);

    std::string absolutePath = absolute(std::filesystem::path(path)).generic_string();
    _file = fopen(absolutePath.c_str(), "rb");
    if (!_file)
        return Error::fromErrno(absolutePath);

    // Disable libc buffering, we manage our own buffer.
    if (setvbuf(_file, nullptr, _IONBF, 0) != 0)
        return Error::fromErrno(absolutePath);

    // Compute file size at open time.
    if (fseeko(_file, 0, SEEK_END) != 0)
        return Error::fromErrno(absolutePath);
    int64_t fileEnd = ftello(_file);
    if (fileEnd == -1)
        return Error::fromErrno(absolutePath);
    if (fseeko(_file, 0, SEEK_SET) != 0)
        return Error::fromErrno(absolutePath);

    _bufSize = bufferSize;
    base_type::open({}, fileEnd, absolutePath);
    return {};
}

Result<size_t> FileInputStream::_underflow(void *data, size_t size, Buffer *buffer) {
    assert(buffer->remaining() == 0);

    if (!_buf)
        _buf = std::make_unique<char[]>(_bufSize);

    if (size < _bufSize) {
        // Small read/skip/refill: fill the internal buffer.
        size_t bytesRead = fread(_buf.get(), 1, _bufSize, _file);
        if (bytesRead == 0 && !feof(_file))
            return Error::fromErrno(displayPath());
        buffer->reset(_buf.get(), _buf.get(), _buf.get() + bytesRead);
        if (data) {
            return buffer->read(data, std::min(size, bytesRead));
        } else {
            return buffer->skip(std::min(size, bytesRead));
        }
    } else if (data) {
        // Large read: direct fread.
        size_t bytesRead = fread(data, 1, size, _file);
        if (bytesRead == 0 && !feof(_file))
            return Error::fromErrno(displayPath());
        return bytesRead;
    } else {
        // Large skip: seek.
        size_t bytesToSkip = std::min(size, this->size() - position());
        if (bytesToSkip > 0 && fseeko(_file, bytesToSkip, SEEK_CUR) != 0)
            return Error::fromErrno(displayPath());
        return bytesToSkip;
    }
}

Result<void> FileInputStream::_close() {
    assert(isOpen());

    int status = fclose(_file);
    _file = nullptr;
    _buf.reset();
    _bufSize = 0;

    Result<void> baseResult = base_type::_close();
    if (status != 0)
        return Error::fromErrno(displayPath());
    return baseResult;
}
