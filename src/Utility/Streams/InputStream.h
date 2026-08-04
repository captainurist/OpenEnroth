#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <optional>
#include <string>
#include <utility>

#include "Utility/Error/Result.h"
#include "Utility/Streams/StreamBuffer.h"

/**
 * Base class for all data input streams.
 *
 * Manages an internal buffer window and implements all read operations on top of it, with inline fast paths
 * for the common case when the requested data is already in the buffer.
 *
 * Data is provided either by setting the buffer directly (for memory-backed streams) or through overriding
 * the `_underflow` virtual method.
 *
 * # Error handling
 *
 * Input streams don't throw. Instead, they have a sticky error state, much like `std::ios` does – the first error
 * that happens is stored inside the stream, and from that point on the stream is dead: all reads return zero bytes,
 * and all subsequent errors are ignored so that the first (most informative) one is preserved.
 *
 * This is what makes deserialization code readable. Deserializers are long sequences of unconditional reads, and
 * having to check every single one of them would drown the actual logic:
 * ```
 * void deserialize(InputStream &src, IndoorDelta_MM7 *dst) {
 *     deserialize(src, &dst->header);
 *     deserialize(src, &dst->visibleOutlines);
 *     deserialize(src, &dst->actors);
 *     // ...20 more lines of exactly this.
 * }
 * ```
 * With a sticky error state, none of these lines need to change. The caller that created the stream checks once,
 * at the end:
 * ```
 * Result<IndoorDelta_MM7> loadDelta(const Blob &blob) {
 *     BlobInputStream stream(blob);
 *     IndoorDelta_MM7 result;
 *     deserialize(stream, &result);
 *     MM_TRY_VOID(stream.check());
 *     return result;
 * }
 * ```
 *
 * Note that a failed read never leaves the caller's buffer uninitialized – whatever couldn't be read is zero-filled,
 * see `read` and `readOrFail`. Deserialized objects are therefore well-defined (if meaningless) even on the error
 * path, so the code that reads them can't trip over uninitialized memory before the error is noticed.
 */
class InputStream {
 public:
    using Buffer = StreamBuffer<const char>;

    virtual ~InputStream();

    /**
     * Note that on an already-failed stream this method zero-fills `data` and returns 0, so that the caller never
     * observes uninitialized memory.
     *
     * @param[out] data                 Output buffer to write read data into.
     * @param size                      Number of bytes to read.
     * @return                          Number of bytes actually read. A return value that's less than `size` signals
     *                                  end of stream, or a failed stream.
     */
    [[nodiscard]] size_t read(void *data, size_t size) {
        assert(isOpen());
        assert(data || size == 0);

        if (size == 0)
            return 0;

        if (failed()) [[unlikely]] {
            memset(data, 0, size);
            return 0;
        }

        if (size <= _buffer.remaining())
            return _buffer.read(data, size);

        return underflow(data, size);
    }

    /**
     * Reads the requested amount of data from the stream, putting the stream into a failed state if unable to do so.
     *
     * On failure the part of `data` that couldn't be read is zero-filled. Check the result with `check` or `failed`.
     *
     * @param[out] data                 Output buffer to write read data into.
     * @param size                      Number of bytes to read.
     */
    void readOrFail(void *data, size_t size) {
        size_t bytesRead = read(data, size);
        if (bytesRead != size) [[unlikely]] {
            memset(static_cast<char *>(data) + bytesRead, 0, size - bytesRead);
            setReadError(size, bytesRead);
        }
    }

    /**
     * Reads everything that's in this stream, writing into the provided string.
     *
     * @param[out] dst                  String to write the data into. Previous contents are cleared.
     * @return                          Number of bytes read from the stream.
     */
    [[nodiscard]] size_t readAll(std::string *dst);

    /**
     * Reads everything that's in this stream.
     *
     * @return                          Data read from the stream, as `std::string`.
     */
    [[nodiscard]] std::string readAll() {
        std::string result;
        (void) readAll(&result);
        return result;
    }

    /**
     * @param size                      Number of bytes to skip.
     * @return                          Number of bytes actually skipped. A return value that's less than `size` signals
     *                                  end of stream, or a failed stream.
     */
    [[nodiscard]] size_t skip(size_t size) {
        assert(isOpen());

        if (failed()) [[unlikely]]
            return 0;

        if (size <= _buffer.remaining())
            return _buffer.skip(size);

        return underflow(nullptr, size);
    }

    /**
     * Same as `readOrFail`, but for skipping bytes.
     *
     * @param size                      Number of bytes to skip.
     */
    void skipOrFail(size_t size) {
        size_t bytesSkipped = skip(size);
        if (bytesSkipped != size) [[unlikely]]
            setSkipError(size, bytesSkipped);
    }

    /**
     * Reads data from the stream until the given delimiter is found, writing into the provided string. The delimiter
     * itself is consumed from the stream but not written to the string.
     *
     * @param delimiter                 Delimiter character to search for.
     * @param[out] dst                  String to write the data into. Previous contents are cleared.
     * @return                          Number of bytes read from the stream, including the delimiter if it was found.
     */
    [[nodiscard]] size_t readUntil(char delimiter, std::string *dst) {
        assert(isOpen());
        assert(dst);
        dst->clear();

        if (failed()) [[unlikely]]
            return 0;

        if (const char *p = static_cast<const char *>(memchr(_buffer.pos(), delimiter, _buffer.remaining()))) {
            size_t bytesRead = _buffer.read(dst, p - _buffer.pos());
            bytesRead += _buffer.skip(1);
            return bytesRead;
        }

        return readUntilSlow(delimiter, dst);
    }

    /**
     * Reads data from the stream until the given delimiter is found and returns it as a string. The delimiter itself
     * is consumed from the stream but not included in the returned string.
     *
     * @param delimiter                 Delimiter character to search for.
     * @return                          Data read from the stream, up to (but not including) the delimiter.
     */
    [[nodiscard]] std::string readUntil(char delimiter) {
        std::string result;
        (void) readUntil(delimiter, &result);
        return result;
    }

    /**
     * Closes this input stream. Reading from a closed stream will result in undefined behavior.
     *
     * Does nothing if the stream is already closed.
     */
    void close() {
        if (isOpen())
            _close(true);
    }

    /**
     * @return                          Whether this stream is open.
     */
    [[nodiscard]] bool isOpen() const { return _isOpen; }

    /**
     * @return                          Whether this stream has failed. A failed stream stays failed until it is
     *                                  reopened.
     */
    [[nodiscard]] bool failed() const { return _error.has_value(); }

    /**
     * @return                          The first error that this stream has encountered. Only valid if `failed`
     *                                  returns `true`.
     */
    [[nodiscard]] const Error &error() const {
        assert(failed());
        return *_error;
    }

    /**
     * The one call that turns a sequence of unchecked reads into a checked operation. See the class-level docs.
     *
     * @return                          Success if this stream hasn't failed, and the first error it has encountered
     *                                  otherwise.
     */
    [[nodiscard]] Result<void> check() const {
        if (failed()) [[unlikely]]
            return std::unexpected(*_error);
        return {};
    }

    /**
     * Puts this stream into a failed state. Does nothing if the stream has already failed – the first error wins,
     * as it is the one that describes what actually went wrong.
     *
     * Intended to be used by the code that reads from the stream, to report semantic errors (e.g. a bad signature)
     * through the same channel that read errors go through.
     *
     * @param error                     Error to store.
     */
    void setFailed(Error error) {
        if (!_error.has_value()) [[likely]]
            _error = std::move(error);
    }

    /**
     * Migration bridge for the code that hasn't been converted to `Result` yet – throws an `Exception` if this
     * stream has failed.
     *
     * @throws Exception                If this stream has failed.
     */
    void checkOrThrow() const;

    /**
     * @return                          Current position in the stream, in bytes from the beginning.
     */
    [[nodiscard]] size_t position() const { return _bufferBase + _buffer.used(); }

    /**
     * @return                          Total size of the stream in bytes, or `size_t(-1)` for unsized streams.
     */
    [[nodiscard]] size_t size() const { return _size; }

    /**
     * @return                          Path to the file or resource being read, to be used for debugging and error
     *                                  reporting.
     */
    [[nodiscard]] const std::string &displayPath() const { return _displayPath; }

 protected:
    InputStream() = default;

    /**
     * Initializes the stream with the given buffer.
     *
     * @param buffer                    Initial buffer state.
     * @param size                      Total stream size in bytes, or `size_t(-1)` if unknown.
     * @param displayPath               Display path for error reporting.
     */
    void open(Buffer buffer, size_t size, std::string_view displayPath);

    /**
     * Fetches more data from the underlying source. Override in subclasses that perform I/O.
     *
     * Three modes of operation:
     * - `size == 0`: just refills the buffer without consuming any data.
     * - `data != nullptr`: reads `size` bytes into `data`.
     * - `data == nullptr && size > 0`: skips `size` bytes.
     *
     * In all modes, sets `*buffer` to the new buffer state.
     *
     * @param[out] data                 Buffer to read into, or `nullptr` for skip/refill.
     * @param size                      Number of bytes to read or skip.
     * @param[out] buffer               New buffer state.
     * @return                          Number of bytes read into `data` or skipped. Implementations should call
     *                                  `setFailed` and return 0 on I/O errors.
     */
    virtual size_t _underflow(void *data, size_t size, Buffer *buffer);

    /**
     * Closes the underlying source, releasing any held resources. Override in subclasses that need cleanup.
     * Derived implementations should call `InputStream::_close()` at the end.
     *
     * @param canReportErrors           Whether the implementation is allowed to report errors through `setFailed`.
     *                                  When called from a destructor via `destroy()`, this is `false` and the
     *                                  implementation should do best-effort cleanup silently.
     */
    virtual void _close(bool canReportErrors);

    /**
     * Silent close for use in derived destructors. Calls `_close` with `canReportErrors=false`.
     */
    void destroy() noexcept {
        if (isOpen())
            _close(false);
    }

    void setReadError(size_t requested, size_t actual);
    void setSkipError(size_t requested, size_t actual);

 private:
    size_t underflow(void *data, size_t size);
    size_t readUntilSlow(char delimiter, std::string *dst);

 private:
    Buffer _buffer;
    size_t _bufferBase = 0;
    size_t _size = static_cast<size_t>(-1);
    bool _isOpen = false;
    std::string _displayPath;
    std::optional<Error> _error;
};
