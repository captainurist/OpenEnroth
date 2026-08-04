#include "ResourceManager.h"

#include "Library/LodFormats/LodFormats.h"
#include "Library/FileSystem/Interface/FileSystem.h"

#include "EngineFileSystem.h"

ResourceManager::ResourceManager() = default;
ResourceManager::~ResourceManager() = default;

Result<void> ResourceManager::open() {
    // TODO(captainurist): the caller should show this to the user as
    //      Error(localization->str(LSTR_MIGHT_AND_MAGIC_VII_IS_HAVING_TROUBLE), localization->str(LSTR_REINSTALL_NECESSARY));
    // but we can't use the localization object here cause it's not yet initialized.
    return _eventsLodReader.open(dfs->read("data/events.lod"));
}

Result<Blob> ResourceManager::eventsData(std::string_view filename) {
    MM_TRY(Blob data, _eventsLodReader.read(filename));
    return lod::decodeMaybeCompressed(data);
}
