#include "MergingFileSystem.h"

#include <memory>
#include <ranges>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "Utility/Error/Result.h"
#include "Library/FileSystem/Interface/FileSystemError.h"
#include "Library/FileSystem/Null/NullFileSystem.h"

MergingFileSystem::MergingFileSystem(std::vector<const FileSystem *> bases) {
    _bases = std::move(bases);
}

MergingFileSystem::~MergingFileSystem() = default;

Result<bool> MergingFileSystem::_exists(FileSystemPathView path) const {
    for (const FileSystem *base : _bases)
        if (co_await base->exists(path))
            co_return true;
    co_return false;
}

Result<FileStat> MergingFileSystem::_stat(FileSystemPathView path) const {
    bool dirFound = false;
    for (const FileSystem *base : _bases) {
        FileStat stat = co_await base->stat(path);
        if (stat.type == FILE_REGULAR)
            co_return stat; // Return the first file found, if any.
        if (stat.type == FILE_DIRECTORY)
            dirFound = true;
    }
    co_return dirFound ? FileStat(FILE_DIRECTORY, 0) : FileStat();
}

Result<void> MergingFileSystem::_ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const {
    std::vector<DirectoryEntry> buffer;

    bool hasOne = false;
    for (const FileSystem *base : _bases) {
        if ((co_await base->stat(path)).type != FILE_DIRECTORY)
            continue;

        // We will error out here if the folder was deleted between stat() and ls() calls. That's probably OK.
        hasOne = true;

        co_await base->ls(path, &buffer);
        std::ranges::move(buffer, std::back_inserter(*entries));
    }

    if (!hasOne && !path.isEmpty())
        co_await fileSystemError(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);

    // Note that we don't need std::stable_sort here b/c no fs-specific data is exposed by the entries.
    std::ranges::sort(*entries);
    auto [tailStart, tailEnd] = std::ranges::unique(*entries);
    entries->erase(tailStart, tailEnd);
}

Result<Blob> MergingFileSystem::_read(FileSystemPathView path) const {
    co_return (co_await locateForReading(path))->read(path);
}

Result<std::unique_ptr<InputStream>> MergingFileSystem::_openForReading(FileSystemPathView path) const {
    co_return (co_await locateForReading(path))->openForReading(path);
}

std::string MergingFileSystem::_displayPath(FileSystemPathView path) const {
    if (_bases.empty())
        return NullFileSystem().displayPath(path); // Empty merging FS is basically a NullFileSystem.

    // TODO(captainurist): This is not ideal, we might want to know ALL merged paths, e.g. see
    //                     ScriptingSystem::_initPackageTable. But the API that we have here doesn't allow that.
    for (const FileSystem *base : _bases)
        if (base->stat(path).valueOr(FileStat()).type != FILE_INVALID)
            return base->displayPath(path);

    return _bases[0]->displayPath(path);
}

Result<const FileSystem *> MergingFileSystem::locateForReading(FileSystemPathView path) const {
    const FileSystem *result = locateForReadingOrNull(path);
    if (result == nullptr)
        return fileSystemError(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
    return result;
}

const FileSystem *MergingFileSystem::locateForReadingOrNull(FileSystemPathView path) const {
    for (const FileSystem *base : _bases)
        if (base->stat(path).valueOr(FileStat()).type == FILE_REGULAR)
            return base;

    return nullptr;
}
