#include "MountingFileSystem.h"

#include <memory>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include "Utility/Error/Result.h"
#include "Library/FileSystem/Interface/FileSystemError.h"
#include "Utility/String/Join.h"

MountingFileSystem::MountingFileSystem(std::string_view displayName) : _displayName(displayName) {}
MountingFileSystem::~MountingFileSystem() = default;

void MountingFileSystem::mount(std::string_view path, FileSystem *fileSystem) {
    mount(FileSystemPath(path), fileSystem);
}

void MountingFileSystem::mount(FileSystemPathView path, FileSystem *fileSystem) {
    _trie.insertOrAssign(path, fileSystem);
}

bool MountingFileSystem::unmount(std::string_view path) {
    return unmount(FileSystemPath(path));
}

bool MountingFileSystem::unmount(FileSystemPathView path) {
    Node *node = _trie.find(path);
    if (!node || !node->hasValue())
        return false; // Should be a real mount point, unmount("") is not equivalent to clearMounts().

    return _trie.erase(node);
}

void MountingFileSystem::clearMounts() {
    _trie.clear();
}

Result<bool> MountingFileSystem::_exists(FileSystemPathView path) const {
    assert(!path.isEmpty());

    auto [node, mount, tail] = walk(path);
    if (node)
        return true;
    return mount ? mount->exists(tail) : Result<bool>(false);
}

Result<FileStat> MountingFileSystem::_stat(FileSystemPathView path) const {
    assert(!path.isEmpty());
    auto [node, mount, tail] = walk(path);
    if (node)
        return FileStat(FILE_DIRECTORY, 0);
    return mount ? mount->stat(tail) : Result<FileStat>(FileStat());
}

Result<void> MountingFileSystem::_ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const {
    auto [node, mount, tail] = walk(path);

    if (!node && !mount)
        return fileSystemError(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);

    if (!node)
        return mount->ls(tail, entries);

    if (!mount) {
        for (const auto &[name, _] : node->children())
            entries->push_back(DirectoryEntry(name, FILE_DIRECTORY));
        return {};
    }

    // Need to merge in this case.
    if (Result<void> status = mount->ls(tail, entries); !status)
        return status;
    std::ranges::sort(*entries);
    size_t originalSize = entries->size();
    bool cleanupNeeded = false;
    for (const auto &[name, _] : node->children()) {
        auto range = std::ranges::equal_range(
            entries->begin(), entries->begin() + originalSize, name, std::ranges::less(), &DirectoryEntry::name);

        size_t size = range.size();

        if (size == 0) {
            entries->push_back(DirectoryEntry(name, FILE_DIRECTORY));
        } else if (size == 1) {
            range[0].type = FILE_DIRECTORY;
        } else {
            assert(size == 2); // Schrodingermaxxed fs, still should not have more than two identical entries.
            range[0].type = FILE_DIRECTORY;
            range[1].type = FILE_INVALID;
            cleanupNeeded = true;
        }
    }
    if (cleanupNeeded)
        std::erase_if(*entries, [] (const DirectoryEntry &entry) { return entry.type == FILE_INVALID; });
    return {};
}

Result<Blob> MountingFileSystem::_read(FileSystemPathView path) const {
    auto [mount, tail] = co_await walkForReading(path);
    co_return mount->read(tail);
}

Result<void> MountingFileSystem::_write(FileSystemPathView path, const Blob &data) {
    auto [mount, tail] = co_await walkForWriting(path);
    co_await mount->write(tail, data);
}

Result<std::unique_ptr<InputStream>> MountingFileSystem::_openForReading(FileSystemPathView path) const {
    auto [mount, tail] = co_await walkForReading(path);
    co_return mount->openForReading(tail);
}

Result<std::unique_ptr<OutputStream>> MountingFileSystem::_openForWriting(FileSystemPathView path) {
    auto [mount, tail] = co_await walkForWriting(path);
    co_return mount->openForWriting(tail);
}

Result<void> MountingFileSystem::_rename(FileSystemPathView srcPath, FileSystemPathView dstPath) {
    auto [srcNode, srcMount, srcTail] = walk(srcPath);
    auto [dstNode, dstMount, dstTail] = walk(dstPath);

    if (srcNode)
        return fileSystemError(this, FS_RENAME_FAILED_SRC_NOT_WRITEABLE, srcPath, dstPath);
    if (dstNode)
        return fileSystemError(this, FS_RENAME_FAILED_DST_IS_DIR, srcPath, dstPath);
    if (!srcMount)
        return fileSystemError(this, FS_RENAME_FAILED_SRC_DOESNT_EXIST, srcPath, dstPath);
    if (!dstMount)
        return fileSystemError(this, FS_RENAME_FAILED_DST_NOT_WRITEABLE, srcPath, dstPath);

    if (srcMount == dstMount) {
        return srcMount->rename(srcTail, dstTail);
    } else {
        // Just forward to recursive copy & remove. Every call will resolve the mount points again and again, so
        // suboptimal, but OK for now.
        return FileSystem::_rename(srcPath, dstPath);
    }
}

Result<bool> MountingFileSystem::_remove(FileSystemPathView path) {
    auto [node, mount, tail] = walk(path);
    if (node)
        return fileSystemError(this, FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, path);
    if (!mount)
        return false; // Nothing to remove.
    return mount->remove(tail);
}

std::string MountingFileSystem::_displayPath(FileSystemPathView path) const {
    // TODO(captainurist): this is not symmetric with that's done in read / openForReading / openForWriting.
    return join(_displayName, "://", path.string());
}

MountingFileSystem::WalkResult MountingFileSystem::walk(FileSystemPathView path) {
    Node *node = _trie.root();
    FileSystem *mount = node->hasValue() ? node->value() : nullptr;
    if (path.isEmpty())
        return {node, mount, {}};

    std::string_view mountChunk;
    for (std::string_view chunk : path.split()) {
        node = node->child(chunk);
        if (!node)
            break;
        if (node->hasValue()) {
            mount = node->value();
            mountChunk = chunk;
        }
    }

    if (mount) {
        return {node, mount, path.split().tailAfter(mountChunk)};
    } else {
        return {node, nullptr, {}};
    }
}

MountingFileSystem::ConstWalkResult MountingFileSystem::walk(FileSystemPathView path) const {
    return const_cast<MountingFileSystem *>(this)->walk(path);
}

Result<std::pair<const FileSystem *, FileSystemPathView>> MountingFileSystem::walkForReading(FileSystemPathView path) const {
    auto [node, mount, tail] = walk(path);
    if (node)
        return fileSystemError(this, FS_READ_FAILED_PATH_IS_DIR, path);
    if (!mount)
        return fileSystemError(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
    return std::pair(static_cast<const FileSystem *>(mount), tail);
}

Result<std::pair<FileSystem *, FileSystemPathView>> MountingFileSystem::walkForWriting(FileSystemPathView path) {
    auto [node, mount, tail] = walk(path);
    if (node)
        return fileSystemError(this, FS_WRITE_FAILED_PATH_IS_DIR, path);
    if (!mount)
        return fileSystemError(this, FS_WRITE_FAILED_PATH_NOT_WRITEABLE, path); // No mount point => can't write.
    return std::pair(mount, tail);
}
