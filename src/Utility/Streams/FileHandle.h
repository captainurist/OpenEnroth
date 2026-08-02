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

    FileHandle() = default;
    ~FileHandle();

    FileHandle(const FileHandle &) = delete;
    FileHandle &operator=(const FileHandle &) = delete;

    /**
     * Opens an existing file for reading. Fails if the path doesn't point to a regular file.
     *
     * @param path                      Absolute UTF-8 path of the file to open.
     * @throws Exception                On error.
     */
    void openForReading(std::string_view path);

    /**
     * Creates or truncates a file, and opens it for writing.
     *
     * @param path                      Absolute UTF-8 path of the file to open.
     * @throws Exception                On error.
     */
    void openForWriting(std::string_view path);

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
     * Queries the current size of the file. This is not cached - callers that need a stable value should sample it
     * once and keep it, like `FileInputStream` does.
     *
     * @return                          Size of the file, in bytes.
     * @throws Exception                On error.
     */
    [[nodiscard]] size_t size() const;

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
     * Moves the file position. Note that seeking past the end of a file is not an error, so it's up to the caller to
     * clamp if that's not what's wanted.
     *
     * @param position                  New position, in bytes from the beginning of the file.
     * @throws Exception                On error.
     */
    void seek(size_t position);

 private:
    void open(std::string_view path, bool forWriting);

 private:
    NativeHandle _handle = INVALID_HANDLE;
    std::string _displayPath;
};
