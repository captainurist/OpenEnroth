#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>
#include <string_view>

#include "Utility/Error/Result.h"
#include "Utility/Memory/Blob.h"
#include "Utility/Streams/StreamBuffer.h"

/**
 * Base class for all data output streams.
 *
 * Manages an internal write buffer and implements all write operations on top of it, with an inline fast path
 * for the common case when the data fits in the buffer.
 *
 * Subclasses provide the actual I/O by overriding `_overflow`, `_flush`, and `_close`.
 */
class OutputStream {
 public:
    using Buffer = StreamBuffer<char>;

    virtual ~OutputStream();

    /**
     * Writes provided data into the output stream.
     *
     * @param data                      Pointer to the data to write.
     * @param size                      Data size.
     * @return                          Success, or an I/O error.
     */
    Result<void> write(const void *data, size_t size) {
        assert(isOpen());
        assert(data || size == 0);

        if (size == 0)
            return {};

        if (size <= _buffer.remaining()) {
            _buffer.write(data, size);
            return {};
        }

        return overflow(data, size);
    }

    /**
     * Writes provided string into the output stream.
     *
     * @param s                         String to write.
     * @return                          Success, or an I/O error.
     */
    Result<void> write(std::string_view s) {
        return write(s.data(), s.size());
    }

    /**
     * Writes provided `Blob` into the output stream.
     *
     * @param blob                      `Blob` to write.
     * @return                          Success, or an I/O error.
     */
    Result<void> write(const Blob &blob) {
        return write(blob.data(), blob.size());
    }

    /**
     * Flushes this output stream, writing any buffered data to the underlying target.
     *
     * @return                          Success, or an I/O error.
     */
    Result<void> flush() {
        assert(isOpen());
        size_t pos = position();
        Result<void> result = _flush(&_buffer);
        _bufferBase = pos - _buffer.used();
        return result;
    }

    /**
     * Closes this output stream. Writing into or flushing a closed stream will result in undefined behavior.
     *
     * Does nothing if the stream is already closed.
     *
     * @return                          Success, or an I/O error.
     */
    Result<void> close() {
        if (!isOpen())
            return {};
        return _close(&_buffer);
    }

    /**
     * @return                          Whether this stream is open.
     */
    [[nodiscard]] bool isOpen() const { return _isOpen; }

    /**
     * @return                          Current position in the stream, in bytes from the beginning.
     */
    [[nodiscard]] size_t position() const { return _bufferBase + _buffer.used(); }

    /**
     * @return                          Same as `position()`, added for API symmetry with `InputStream`.
     */
    [[nodiscard]] size_t size() const { return position(); }

    /**
     * @return                          Path to the file or resource being written, to be used for debugging and error
     *                                  reporting.
     */
    [[nodiscard]] const std::string &displayPath() const { return _displayPath; }

 protected:
    OutputStream() = default;

    /**
     * Re-initializes the stream with a pre-allocated buffer.
     *
     * @param buffer                    Initial buffer state.
     * @param displayPath               Display path for error reporting.
     */
    void open(Buffer buffer, std::string_view displayPath);

    /**
     * Called when a write doesn't fit in the current buffer. Implementations should handle the overflow data
     * (write it out or store it), and provide a new writable buffer via the out parameter.
     *
     * @param[in,out] buffer            Current buffer state on input. Set to the new buffer state on output. Data in
     *                                  `[buffer->start, buffer->pos)` is treated as dirty (not yet flushed).
     * @param data                      Pointer to the overflow data to write.
     * @param size                      Size of the overflow data, always greater than `buffer->remaining()`.
     * @return                          Success, or an I/O error.
     */
    virtual Result<void> _overflow(Buffer *buffer, const void *data, size_t size) = 0;

    /**
     * Flushes buffered data to the underlying target.
     *
     * @param[in,out] buffer            Current buffer state.
     * @return                          Success, or an I/O error.
     */
    virtual Result<void> _flush(Buffer *buffer) = 0;

    /**
     * Flushes any remaining buffered data and releases held resources.
     *
     * Derived implementations should call `OutputStream::_close()` at the end.
     *
     * @param[in,out] buffer            Current buffer state.
     * @return                          Success, or an I/O error.
     */
    virtual Result<void> _close(Buffer *buffer) = 0;

    /**
     * Non-throwing close for use in derived destructors. Errors are explicitly dropped - there's nothing a
     * destructor could do with them.
     */
    void destroy() noexcept {
        if (isOpen())
            _close(&_buffer).discard();
    }

 private:
    Result<void> overflow(const void *data, size_t size);

 private:
    Buffer _buffer;
    size_t _bufferBase = 0;
    bool _isOpen = false;
    std::string _displayPath;
};
