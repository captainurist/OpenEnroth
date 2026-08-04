#include "MaskingFileSystem.h"

#include <memory>
#include <string>
#include <vector>

#include "Utility/Error/Result.h"
#include "Library/FileSystem/Interface/FileSystemError.h"
#include "Utility/String/Join.h"

MaskingFileSystem::MaskingFileSystem(FileSystem *base) : ProxyFileSystem(base) {}

MaskingFileSystem::~MaskingFileSystem() = default;

void MaskingFileSystem::mask(std::string_view path) {
    mask(FileSystemPath(path));
}

void MaskingFileSystem::mask(FileSystemPathView path) {
    _masks.insertOrAssign(path, true);
}

bool MaskingFileSystem::unmask(std::string_view path) {
    return unmask(FileSystemPath(path));
}

bool MaskingFileSystem::unmask(FileSystemPathView path) {
    FileSystemTrieNode<bool> *node = _masks.find(path);
    if (!node || !node->hasValue() || !node->value())
        return false; // Can only unmask what was previously masked.
    node->value() = false;
    return true;
}

void MaskingFileSystem::clearMasks() {
    _masks.clear();
}

bool MaskingFileSystem::isMasked(FileSystemPathView path) const {
    const FileSystemTrieNode<bool> *node = _masks.root();

    for (std::string_view chunk : path.split()) {
        if (node->hasValue() && node->value())
            return true;

        node = node->child(chunk);
        if (!node)
            return false;
    }

    return node->hasValue() && node->value();
}

Result<bool> MaskingFileSystem::_exists(FileSystemPathView path) const {
    if (isMasked(path))
        return false;
    return ProxyFileSystem::_exists(path);
}

Result<FileStat> MaskingFileSystem::_stat(FileSystemPathView path) const {
    if (isMasked(path))
        return FileStat();
    return ProxyFileSystem::_stat(path);
}

Result<void> MaskingFileSystem::_ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const {
    if (isMasked(path)) {
        if (path.isEmpty()) {
            return {}; // Pretend root exists even if it was masked.
        } else {
            return fileSystemError(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);
        }
    }

    if (Result<void> status = ProxyFileSystem::_ls(path, entries); !status)
        return status;

    if (const FileSystemTrieNode<bool> *node = _masks.find(path)) {
        std::erase_if(*entries, [node] (const DirectoryEntry &entry) {
            if (FileSystemTrieNode<bool> *child = node->child(entry.name)) {
                return child->hasValue() && child->value();
            } else {
                return false;
            }
        });
    }
    return {};
}

Result<Blob> MaskingFileSystem::_read(FileSystemPathView path) const {
    if (isMasked(path))
        return fileSystemError(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
    return ProxyFileSystem::_read(path);
}

Result<void> MaskingFileSystem::_write(FileSystemPathView path, const Blob &data) {
    if (isMasked(path))
        return fileSystemError(this, FS_WRITE_FAILED_PATH_NOT_WRITEABLE, path);
    return ProxyFileSystem::_write(path, data);
}

Result<std::unique_ptr<InputStream>> MaskingFileSystem::_openForReading(FileSystemPathView path) const {
    if (isMasked(path))
        return fileSystemError(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
    return ProxyFileSystem::_openForReading(path);
}

Result<std::unique_ptr<OutputStream>> MaskingFileSystem::_openForWriting(FileSystemPathView path) {
    if (isMasked(path))
        return fileSystemError(this, FS_WRITE_FAILED_PATH_NOT_WRITEABLE, path);
    return ProxyFileSystem::_openForWriting(path);
}

Result<void> MaskingFileSystem::_rename(FileSystemPathView srcPath, FileSystemPathView dstPath) {
    if (isMasked(srcPath))
        return fileSystemError(this, FS_RENAME_FAILED_SRC_DOESNT_EXIST, srcPath, dstPath);
    if (isMasked(dstPath))
        return fileSystemError(this, FS_RENAME_FAILED_DST_NOT_WRITEABLE, srcPath, dstPath);
    return ProxyFileSystem::_rename(srcPath, dstPath);
}

Result<bool> MaskingFileSystem::_remove(FileSystemPathView path) {
    if (isMasked(path))
        return false;
    return ProxyFileSystem::_remove(path);
}

std::string MaskingFileSystem::_displayPath(FileSystemPathView path) const {
    if (isMasked(path))
        return join("masked://", path.string());
    return ProxyFileSystem::_displayPath(path);
}
