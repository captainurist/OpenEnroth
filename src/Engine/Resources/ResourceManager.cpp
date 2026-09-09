#include "ResourceManager.h"

#include <string>
#include <string_view>

#include "Library/LodFormats/LodFormats.h"
#include "Library/FileSystem/Interface/FileSystem.h"

#include "EngineFileSystem.h"

namespace {

struct EncodingMarker {
    TextEncoding encoding;
    std::string_view word; // Word for "sword", spelled & encoded the way `encoding`'s localization spells it.
};

/**
 * Every localization ships the same `items.txt`, and every one of them has swords in it. So the word for "sword",
 * in the localization's own encoding, identifies the encoding.
 *
 * Charset detection doesn't work here. English game data is 99.98% ASCII, and the handful of non-ASCII bytes it
 * does have are punctuation that windows-1251 and windows-1252 map the same way - there's simply nothing to detect.
 *
 * Note that the English word can't be used as a marker: the localized files keep an English "Skill Group" column,
 * so "sword" is in all of them. English is what we fall back to instead.
 */
constexpr EncodingMarker encodingMarkers[] = {
    {ENCODING_WINDOWS_1251, "\xEC\xE5\xF7"},        // "меч", Russian.
    {ENCODING_WINDOWS_1251, "\xCC\xE5\xF7"},        // "Меч".
    {ENCODING_WINDOWS_1252, "\xE9p\xE9\x65"},       // "épée", French.
    {ENCODING_WINDOWS_1252, "\xC9p\xE9\x65"},       // "Épée".
    {ENCODING_WINDOWS_1252, "schwert"},             // "schwert", German.
    {ENCODING_WINDOWS_1252, "Schwert"},
};

} // namespace

ResourceManager::ResourceManager() = default;
ResourceManager::~ResourceManager() = default;

void ResourceManager::open() {
    _eventsLodReader.open(dfs->read("data/events.lod"));
    // TODO(captainurist):
    //  on exception:
    //      Error(localization->str(LSTR_MIGHT_AND_MAGIC_VII_IS_HAVING_TROUBLE), localization->str(LSTR_REINSTALL_NECESSARY));
    //  but we can't use localization object here cause it's not yet initialized.

    Blob items = eventsData("items.txt");
    std::string_view itemsText = items.str();

    _gameDataEncoding = ENCODING_WINDOWS_1252;
    for (const EncodingMarker &marker : encodingMarkers) {
        if (itemsText.contains(marker.word)) {
            _gameDataEncoding = marker.encoding;
            break;
        }
    }
}

Blob ResourceManager::eventsData(std::string_view filename) {
    return lod::decodeMaybeCompressed(_eventsLodReader.read(filename));
}

std::string ResourceManager::eventsText(std::string_view filename) {
    return txt::encodedToUtf8(eventsData(filename).str(), _gameDataEncoding);
}
