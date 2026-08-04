#include "SndReader.h"

#include <algorithm>
#include <utility>
#include <unordered_map>
#include <string>
#include <vector>

#include "Library/Compression/Compression.h"
#include "Library/Logger/Logger.h"
#include "Library/Snapshots/SnapshotSerialization.h"

#include "Utility/Streams/BlobInputStream.h"
#include "Utility/String/Ascii.h"
#include "Utility/Error/Result.h"

#include "SndSnapshots.h"

SndReader::SndReader() = default;

SndReader::~SndReader() = default;

Result<void> SndReader::open(std::string_view path) {
    close();
    // TODO(captainurist): #exceptions Blob::fromFile still throws, drop the tryCatch once it's ported.
    MM_TRY(Blob blob, tryCatch([&] { return Blob::fromFile(path); }));
    return open(std::move(blob));
}

Result<void> SndReader::open(Blob blob) {
    BlobInputStream stream(blob);

    std::vector<SndEntry> entries;
    MM_TRY_VOID(deserialize(stream, &entries, tags::each, tags::via<SndEntry_MM7>)
                    .withContext("File '{}' is not a valid SND", blob.displayPath()));

    std::unordered_map<std::string, SndEntry> files;
    for (SndEntry &entry : entries) {
        std::string name = ascii::toLower(entry.name);
        if (files.contains(name))
            return fail("File '{}' is not a valid SND: contains duplicate entries for '{}'", blob.displayPath(), name);

        if (entry.offset + entry.size > blob.size())
            return fail("File '{}' is not a valid SND: entry '{}' points outside the SND file", blob.displayPath(), entry.name);

        files.emplace(std::move(name), std::move(entry));
    }

    // All good, this is a valid SND, can update `this`.
    _snd = std::move(blob);
    _files = std::move(files);
    return {};
}

void SndReader::close() {
    // Double-closing is OK.
    _snd = Blob();
    _files = {};
}

bool SndReader::exists(std::string_view filename) const {
    assert(isOpen());

    return _files.contains(ascii::toLower(filename));
}

Result<Blob> SndReader::read(std::string_view filename) const {
    assert(isOpen());

    const auto pos = _files.find(ascii::toLower(filename));
    if (pos == _files.cend())
        return fail("Entry '{}' doesn't exist in SND file '{}'", filename, _snd.displayPath());
    const SndEntry &entry = pos->second;

    Blob result = _snd.subBlob(entry.offset, entry.size);
    std::string path = fmt::format("{}/{}", _snd.displayPath(), filename);
    if (entry.decompressedSize && entry.decompressedSize != entry.size) {
        Blob compressed = result.withDisplayPath(path);
        Result<Blob> uncompressed = zlib::uncompress(compressed, entry.decompressedSize);
        if (!uncompressed) {
            // Some of the SNDs shipped with the vanilla games have corrupt checksums, try to recover what we can.
            result = zlib::uncompressBestEffort(compressed, entry.decompressedSize);
            if (!result)
                return std::move(uncompressed).withContext("SndReader: failed to decompress '{}'", path);
            logger->warning("SndReader: '{}' has corrupt checksum, recovered {} of {} expected bytes",
                            path, result.size(), entry.decompressedSize);
        } else {
            result = *std::move(uncompressed);
        }
    }
    return result.withDisplayPath(path);
}

std::vector<std::string> SndReader::ls() const {
    assert(isOpen());

    std::vector<std::string> result;
    for (const auto &[name, _] : _files)
        result.push_back(name);
    std::sort(result.begin(), result.end());
    return result;
}

bool snd::detect(const Blob &data) {
    if (data.size() < 4)
        return false;

    BlobInputStream stream(data);

    uint32_t entryCount;
    if (!deserialize(stream, &entryCount))
        return false;
    if (entryCount == 0)
        return false; // Empty snd file is not valid.

    size_t headerSize = 4 + entryCount * sizeof(SndEntry_MM7);
    if (data.size() < headerSize)
        return false;

    // Just check up to 16 entries and we're good.
    for (size_t i = 0, count = std::min<size_t>(entryCount, 16); i < count; i++) {
        SndEntry_MM7 entry;
        if (!deserialize(stream, &entry))
            return false;

        if (entry.offset < headerSize)
            return false;

        if (static_cast<size_t>(entry.offset) + static_cast<size_t>(entry.size) > data.size())
            return false;

        // For compressed entries, zlib's max compression ratio is 1032:1, so anything beyond that is clearly garbage.
        // See https://zlib.net/zlib_tech.html.
        if (entry.size != 0 && entry.decompressedSize > static_cast<size_t>(entry.size) * 1032)
            return false;
    }

    return true;
}
