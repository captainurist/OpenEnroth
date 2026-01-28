#pragma once

#include <string>

#include "Utility/Embedded.h"
#include "Utility/Memory/Blob.h"

#include "StringOutputStream.h"

class BlobOutputStream : public OutputStream {
 public:
    BlobOutputStream();
    explicit BlobOutputStream(Blob *target, std::string_view displayPath = {});
    virtual ~BlobOutputStream();

    void open(Blob *target, std::string_view displayPath = {});

    virtual void write(const void *data, size_t size) override;
    virtual void flush() override;
    virtual void close() override;
    [[nodiscard]] virtual std::string displayPath() const override;

    using OutputStream::write;

 private:
    void closeInternal();

 private:
    Blob *_target = nullptr;
    std::string _buffer;
    std::string _displayPath;
};
