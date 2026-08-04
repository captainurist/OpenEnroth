#pragma once

#include "Utility/Error/Result.h"
#include <string_view>
#include <string>
#include <memory>
#include <map>

#include "Utility/Streams/OutputStream.h"
#include "Utility/Memory/Blob.h"

#include "LodInfo.h"

class LodWriter {
 public:
    LodWriter();
    LodWriter(OutputStream *stream, LodInfo info);
    ~LodWriter();

    Result<void> open(std::string_view path, LodInfo info);
    void open(OutputStream *stream, LodInfo info);

    /**
     * Writes out the LOD index & all buffered entries, and closes this writer.
     *
     * @return                          Success, or an I/O error.
     */
    Result<void> close();

    [[nodiscard]] bool isOpen() const {
        return _stream != nullptr;
    }

    void write(std::string_view filename, const Blob &data);
    void write(std::string_view filename, Blob &&data);

 private:
    std::unique_ptr<OutputStream> _ownedStream;
    OutputStream *_stream = nullptr;
    LodInfo _info;
    std::map<std::string, Blob> _files; // Having this one sorted makes implementation simpler.
};
