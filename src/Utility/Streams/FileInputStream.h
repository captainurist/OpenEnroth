#pragma once

#include <cstdio>
#include <memory>
#include <string_view>

#include "InputStream.h"

// TODO(captainurist): just use raw file io, not FILE*

/**
 * Input stream that reads from a file.
 */
class FileInputStream : public InputStream {
    using base_type = InputStream;

 public:
    static constexpr size_t DEFAULT_BUFFER_SIZE = 1024 * 1024;

    FileInputStream() = default;
    virtual ~FileInputStream();

    /**
     * Opens a file for reading.
     *
     * @param path                      Path to the file to open.
     * @param bufferSize                Size of the internal read buffer.
     * @return                          Success, or an error if the file couldn't be opened.
     */
    Result<void> open(std::string_view path, size_t bufferSize = DEFAULT_BUFFER_SIZE);

 private:
    virtual Result<size_t> _underflow(void *data, size_t size, Buffer *buffer) override;
    virtual Result<void> _close() override;

 private:
    FILE *_file = nullptr;
    std::unique_ptr<char[]> _buf;
    size_t _bufSize = 0;
};
