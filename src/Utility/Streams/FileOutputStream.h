#pragma once

#include <cstdio>
#include <memory>
#include <string_view>

#include "OutputStream.h"

// TODO(captainurist): just use raw file io, not FILE*

/**
 * Output stream that writes to a file.
 */
class FileOutputStream : public OutputStream {
    using base_type = OutputStream;

 public:
    static constexpr size_t DEFAULT_BUFFER_SIZE = 1024 * 1024;

    FileOutputStream() = default;
    virtual ~FileOutputStream();

    /**
     * Opens a file for writing.
     *
     * @param path                      Path to the file to open.
     * @param bufferSize                Size of the internal write buffer.
     * @return                          Success, or an error if the file couldn't be opened.
     */
    Result<void> open(std::string_view path, size_t bufferSize = DEFAULT_BUFFER_SIZE);

 private:
    virtual Result<void> _overflow(Buffer *buffer, const void *data, size_t size) override;
    virtual Result<void> _flush(Buffer *buffer) override;
    virtual Result<void> _close(Buffer *buffer) override;

    Result<void> writeBuffer(const Buffer &buffer);

 private:
    FILE *_file = nullptr;
    std::unique_ptr<char[]> _buf;
    size_t _bufSize = 0;
};
