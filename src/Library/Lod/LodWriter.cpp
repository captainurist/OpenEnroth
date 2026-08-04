#include "LodWriter.h"

#include <utility>
#include <vector>
#include <memory>

#include "Library/Serialization/Serialization.h"
#include "Library/Snapshots/SnapshotSerialization.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/String/Ascii.h"

#include "LodSnapshots.h"

LodWriter::LodWriter() {}

LodWriter::LodWriter(OutputStream *stream, LodInfo info) {
    open(stream, std::move(info));
}

LodWriter::~LodWriter() {
    close().discard(); // Nothing a destructor can do about a failed write - use close() explicitly to handle errors.
}

Result<void> LodWriter::open(std::string_view path, LodInfo info) {
    auto ownedStream = std::make_unique<FileOutputStream>();
    co_await ownedStream->open(path); // If this fails, no field is overwritten.
    open(ownedStream.get(), std::move(info));
    _ownedStream = std::move(ownedStream);
}

void LodWriter::open(OutputStream *stream, LodInfo info) {
    assert(stream);

    close().discard(); // Opening over an unclosed writer drops any pending write errors - close explicitly to handle them.

    _stream = stream;
    _info = std::move(info);
}

Result<void> LodWriter::close() {
    if (!isOpen())
        co_return; // Double-closing is OK.

    // Write out LOD header.
    LodHeader header;
    header.signature = "LOD";
    header.version = toString(_info.version);
    header.description = _info.description;
    header.numDirectories = 1;
    co_await serialize(header, _stream, tags::via<LodHeader_MM6>);

    // Write out root entry.
    size_t dataSize = 0;
    for (const auto &[_, data] : _files)
        dataSize += data.size();
    size_t indexSize = _files.size() * fileEntrySize(_info.version);

    LodEntry directoryEntry;
    directoryEntry.name = _info.rootName;
    directoryEntry.dataOffset = sizeof(LodHeader_MM6) + sizeof(LodEntry_MM6);
    directoryEntry.dataSize = indexSize + dataSize;
    directoryEntry.numItems = _files.size();
    co_await serialize(directoryEntry, _stream, tags::via<LodEntry_MM6>);

    // Write out file entries.
    size_t currentOffset = indexSize;
    std::vector<LodEntry> fileEntries;
    for (const auto &[name, data] : _files) {
        LodEntry &entry = fileEntries.emplace_back();
        entry.name = name;
        entry.dataOffset = currentOffset;
        entry.dataSize = data.size();
        entry.numItems = 0;

        currentOffset += data.size();
    }

    if (_info.version == LOD_VERSION_MM8) {
        co_await serialize(fileEntries, _stream, tags::unsized, tags::each, tags::via<LodFileEntry_MM8>);
    } else {
        co_await serialize(fileEntries, _stream, tags::unsized, tags::each, tags::via<LodEntry_MM6>);
    }

    for (const auto &[_, data] : _files)
        co_await _stream->write(data);

    // Close shop.
    _files.clear(); // Important to release the Blobs first, as they might point into a file that we're about to overwrite...
    _ownedStream = {}; // ...here.
    _stream = {};
    _info = {};
}

void LodWriter::write(std::string_view filename, const Blob &data) {
    write(filename, Blob::share(data));
}

void LodWriter::write(std::string_view filename, Blob &&data) {
    assert(isOpen());

    _files.insert_or_assign(ascii::toLower(filename), std::move(data));
}
