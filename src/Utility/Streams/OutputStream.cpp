#include "OutputStream.h"

#include <cassert>

OutputStream::~OutputStream() = default;

void OutputStream::open(Buffer buffer, std::string_view displayPath) {
    _buffer = buffer;
    _bufferBase = 0;
    _isOpen = true;
    _displayPath = displayPath;
}

Result<void> OutputStream::_close(Buffer *buffer) {
    assert(isOpen());
    _buffer.reset(nullptr, nullptr, nullptr);
    _bufferBase = 0;
    _isOpen = false;
    _displayPath = {};
    return {};
}


Result<void> OutputStream::overflow(const void *data, size_t size) {
    assert(size > _buffer.remaining());
    size_t pos = position();
    Result<void> result = _overflow(&_buffer, data, size);
    if (!result) [[unlikely]]
        return result;
    _bufferBase = pos + size - _buffer.used();
    return {};
}
