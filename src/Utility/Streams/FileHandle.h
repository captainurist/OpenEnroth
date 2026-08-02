#pragma once

#include <cstddef>
#include <string>
#include <string_view>

/**
 * RAII wrapper around a native OS file handle - a file descriptor on POSIX, a `HANDLE` on Windows.
 *
 * This is an implementation detail of `FileInputStream` / `FileOutputStream`, and is the only place in the codebase
 * that talks to the OS file API directly. Use the stream classes, or `FileSystem`, instead.
 *
 * All IO operations throw on error, and loop internally over short and interrupted transfers.
 */
class FileHandle {
 public:
#ifdef _WINDOWS
    using NativeHandle = void *; // This is a `HANDLE`, but we don't want to drag `<Windows.h>` into a header.
    static constexpr NativeHandle INVALID_HANDLE = nullptr; // `CreateFileW` never returns `NULL` on success.
#else
    using NativeHandle = int;
    static constexpr NativeHandle INVALID_HANDLE = -1; // Note that `0` is `stdin`, and thus is a valid handle.
#endif

    /**
     * Largest chunk that can be passed to a single OS-level read / write. `ReadFile` / `WriteFile` on Windows take
     * a `DWORD`, and Linux caps a single `read` / `write` at slightly under 2Gb.
     */
    static constexpr size_t MAX_IO_CHUNK_SIZE = 0x7ffff000;

    FileHandle() = default;
    ~FileHandle();

    FileHandle(const FileHandle &) = delete;
    FileHandle &operator=(const FileHandle &) = delete;

    /**
     * Opens an existing file for reading. Fails if the path doesn't point to a regular file.
     *
     * @param path                      Absolute UTF-8 path of the file to open, also used for error reporting.
     * @param maxChunkSize              Largest chunk to pass to a single OS-level read, see `MAX_IO_CHUNK_SIZE`.
     *                                  Lowering this is only useful in tests, as it's the only way to make the
     *                                  chunking loop run on a small file.
     * @throws Exception                On error.
     */
    void openForReading(std::string_view path, size_t maxChunkSize = MAX_IO_CHUNK_SIZE);

    /**
     * Creates or truncates a file, and opens it for writing.
     *
     * @param path                      Absolute UTF-8 path of the file to open, also used for error reporting.
     * @param maxChunkSize              Largest chunk to pass to a single OS-level write, see `openForReading`.
     * @throws Exception                On error.
     */
    void openForWriting(std::string_view path, size_t maxChunkSize = MAX_IO_CHUNK_SIZE);

    /**
     * Closes this handle. Never throws - the handle is always released, and closing errors are reported through the
     * return value so that this can be called from destructors and other non-throwing cleanup paths.
     *
     * Does nothing if this handle is already closed.
     *
     * @return                          OS error code, or `0` on success.
     */
    int close();

    /**
     * @return                          Whether this handle is open.
     */
    [[nodiscard]] bool isOpen() const { return _handle != INVALID_HANDLE; }

    /**
     * @return                          Size of the file as sampled at open time. Always `0` for write handles.
     */
    [[nodiscard]] size_t size() const { return _size; }

    /**
     * @return                          Current OS file offset, tracked internally, without a syscall.
     */
    [[nodiscard]] size_t offset() const { return _offset; }

    /**
     * @return                          Saturating `size() - offset()`. Only meaningful for read handles.
     */
    [[nodiscard]] size_t bytesLeft() const { return _size > _offset ? _size - _offset : 0; }

    /**
     * @return                          Path this handle was opened with, to be used for debugging and error reporting.
     */
    [[nodiscard]] const std::string &displayPath() const { return _displayPath; }

    /**
     * @param[out] data                 Buffer to read into.
     * @param size                      Number of bytes to read.
     * @return                          Number of bytes actually read. A value less than `size` means end of file.
     * @throws Exception                On error.
     */
    size_t read(void *data, size_t size);

    /**
     * Writes all of the provided data out.
     *
     * If this call throws then an unspecified prefix of the data has already been written out, and the caller must
     * not retry the write.
     *
     * @param data                      Data to write.
     * @param size                      Number of bytes to write.
     * @throws Exception                On error.
     */
    void write(const void *data, size_t size);

    /**
     * Moves the file offset forward.
     *
     * @param size                      Number of bytes to seek forward by.
     * @throws Exception                On error.
     */
    void seekForward(size_t size);

 private:
    void open(std::string_view path, bool forWriting, size_t maxChunkSize);

 private:
    NativeHandle _handle = INVALID_HANDLE;
    size_t _size = 0;
    size_t _offset = 0;
    size_t _maxChunkSize = MAX_IO_CHUNK_SIZE;
    std::string _displayPath;
};
