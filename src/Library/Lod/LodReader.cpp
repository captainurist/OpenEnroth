#include "LodReader.h"

#include <cassert>
#include <utility>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <vector>

#include "Library/Compression/Compression.h"
#include "Library/Snapshots/SnapshotSerialization.h"

#include "Utility/Streams/BlobInputStream.h"
#include "Utility/Error/Result.h"
#include "Utility/String/Ascii.h"

#include "LodSnapshots.h"
#include "LodEnums.h"

static Result<LodHeader> parseHeader(InputStream &stream, LodVersion *version) {
    LodHeader header;
    co_await deserialize(stream, &header, tags::via<LodHeader_MM6>);

    if (header.signature != "LOD")
        co_await fail("File '{}' is not a valid LOD: expected signature '{}', got '{}'", stream.displayPath(), "LOD", ascii::toPrintable(header.signature));

    if (!tryDeserialize(header.version, version))
        co_await fail("File '{}' is not a valid LOD: version '{}' is not recognized", stream.displayPath(), ascii::toPrintable(header.version));

    // While LOD structure itself support multiple directories, all LOD files associated with
    // vanilla MM6/7/8 games use a single directory.
    if (header.numDirectories != 1)
        co_await fail("File '{}' is not a valid LOD: expected a single directory, got '{}' directories", stream.displayPath(), header.numDirectories);

    co_return header;
}

static Result<LodEntry> parseDirectoryEntry(InputStream &stream, LodVersion version, size_t lodSize) {
    LodEntry result;
    co_await deserialize(stream, &result, tags::via<LodEntry_MM6>);

    size_t expectedDataSize = result.numItems * fileEntrySize(version);
    if (result.dataSize < expectedDataSize)
        co_await fail("File '{}' is not a valid LOD: invalid root directory index size, expected at least {} bytes, got {} bytes", stream.displayPath(), expectedDataSize, result.dataSize);

    if (result.dataOffset + result.dataSize > lodSize)
        co_await fail("File '{}' is not a valid LOD: root directory index points outside the LOD file", stream.displayPath());

    co_return result;
}

static Result<std::vector<LodEntry>> parseFileEntries(InputStream &stream, const LodEntry &directoryEntry, LodVersion version) {
    std::vector<LodEntry> result;
    if (version == LOD_VERSION_MM8) {
        co_await deserialize(stream, &result, tags::presized(directoryEntry.numItems), tags::each, tags::via<LodFileEntry_MM8>);
    } else {
        co_await deserialize(stream, &result, tags::presized(directoryEntry.numItems), tags::each, tags::via<LodEntry_MM6>);
    }

    for (const LodEntry &entry : result) {
        if (entry.numItems != 0)
            co_await fail("File '{}' is not a valid LOD: subdirectories are not supported, but '{}' is a subdirectory", stream.displayPath(), entry.name);
        if (entry.dataOffset + entry.dataSize > directoryEntry.dataSize)
            co_await fail("File '{}' is not a valid LOD: entry '{}' points outside the LOD file", stream.displayPath(), entry.name);
    }

    co_return result;
}


LodReader::LodReader() = default;

LodReader::~LodReader() {
    close();
}

Result<void> LodReader::open(std::string_view path, LodOpenFlags openFlags) {
    Blob blob = co_await Blob::fromFile(path);
    co_await open(std::move(blob), openFlags);
}

Result<void> LodReader::open(Blob blob, LodOpenFlags openFlags) {
    close();

    size_t expectedSize = sizeof(LodHeader_MM6) + sizeof(LodEntry_MM6); // Header + directory entry.
    if (blob.size() < expectedSize)
        co_await fail("File '{}' is not a valid LOD: expected file size at least {} bytes, got {} bytes", blob.displayPath(), expectedSize, blob.size());

    BlobInputStream lodStream(blob);
    LodVersion version = LOD_VERSION_MM6;
    LodHeader header = co_await parseHeader(lodStream, &version);
    LodEntry rootEntry = co_await parseDirectoryEntry(lodStream, version, blob.size());

    // LODs that come with the Russian version of MM7 are broken.
    rootEntry.dataSize = blob.size() - rootEntry.dataOffset;

    BlobInputStream dirStream(blob.subBlob(rootEntry.dataOffset, rootEntry.dataSize));
    std::vector<LodEntry> entries = co_await parseFileEntries(dirStream, rootEntry, version);

    std::unordered_map<std::string, LodRegion> files;
    for (const LodEntry &entry : entries) {
        std::string name = ascii::toLower(entry.name);
        if (files.contains(name)) {
            if (openFlags & LOD_ALLOW_DUPLICATES) {
                continue; // Only the first entry is kept in this case.
            } else {
                co_await fail("File '{}' is not a valid LOD: contains duplicate entries for '{}'", blob.displayPath(), name);
            }
        }

        LodRegion region;
        region.offset = rootEntry.dataOffset + entry.dataOffset;
        region.size = entry.dataSize;
        files.emplace(std::move(name), region);
    }

    // All good, this is a valid LOD, can update `this`.
    _lod = std::move(blob);
    _info.version = version;
    _info.description = std::move(header.description);
    _info.rootName = std::move(rootEntry.name);
    _files = std::move(files);
}

void LodReader::close() {
    // Double-closing is OK.
    _lod = Blob();
    _info = {};
    _files = {};
}

bool LodReader::exists(std::string_view filename) const {
    assert(isOpen());

    return _files.contains(ascii::toLower(filename));
}

Result<Blob> LodReader::read(std::string_view filename) const {
    assert(isOpen());

    const auto pos = _files.find(ascii::toLower(filename));
    if (pos == _files.cend())
        return fail("Entry '{}' doesn't exist in LOD file '{}'", filename, _lod.displayPath());

    return _lod.subBlob(pos->second.offset, pos->second.size).withDisplayPath(displayPath(filename));
}

std::string LodReader::displayPath(std::string_view filename) const {
    return fmt::format("{}/{}", _lod.displayPath(), filename);
}

std::vector<std::string> LodReader::ls() const {
    assert(isOpen());

    std::vector<std::string> result;
    for (const auto &[name, _] : _files)
        result.push_back(name);
    std::sort(result.begin(), result.end());
    return result;
}

[[nodiscard]] const LodInfo &LodReader::info() const {
    assert(isOpen());

    return _info;
}

bool lod::detect(const Blob &data) {
    if (data.size() < sizeof(LodHeader_MM6) + sizeof(LodEntry_MM6)) // Header + directory entry.
        return false;

    BlobInputStream stream(data);
    LodHeader header;
    if (!deserialize(stream, &header, tags::via<LodHeader_MM6>))
        return false;

    if (header.signature != "LOD")
        return false;

    LodVersion version;
    if (!tryDeserialize(header.version, &version))
        return false;

    // While LOD structure itself support multiple directories, all LOD files associated with
    // vanilla MM6/7/8 games use a single directory.
    if (header.numDirectories != 1)
        return false;

    return true;
}
