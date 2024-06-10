#include "MaskingFileSystem.h"

#include <cassert>
#include <vector>
#include <memory>

#include "Library/FileSystem/Interface/FileSystemException.h"

MaskingFileSystem::MaskingFileSystem(FileSystem *base) : _base(base) {
    assert(_base);
}

MaskingFileSystem::~MaskingFileSystem() = default;

void MaskingFileSystem::mask(std::string_view path) {
    mask(FileSystemPath(path));
}

void MaskingFileSystem::mask(const FileSystemPath &path) {
    // Chop drops all children for the newly inserted node. This is needed for the implementation of `isMasked` to work.
    _masks.chop(_masks.insertOrAssign(path, true));
}

void MaskingFileSystem::clearMasks() {
    _masks.clear();
}

bool MaskingFileSystem::isMasked(const FileSystemPath &path) const {
    return _masks.walk(path)->hasValue();
}

bool MaskingFileSystem::_exists(const FileSystemPath &path) const {
    assert(!path.isEmpty());
    if (isMasked(path))
        return false;
    return _base->exists(path);
}

FileStat MaskingFileSystem::_stat(const FileSystemPath &path) const {
    assert(!path.isEmpty());
    if (isMasked(path))
        return {};
    return _base->stat(path);
}

std::vector<DirectoryEntry> MaskingFileSystem::_ls(const FileSystemPath &path) const {
    if (isMasked(path)) {
        if (path.isEmpty()) {
            return {}; // Pretend root exists even if it was masked.
        } else {
            throw FileSystemException(FileSystemException::LS_FAILED_PATH_DOESNT_EXIST, path);
        }
    }

    std::vector<DirectoryEntry> result = _base->ls(path);

    if (const FileSystemTrieNode<bool> *node = _masks.find(path)) {
        std::erase_if(result, [node] (const DirectoryEntry &entry) {
            if (FileSystemTrieNode<bool> *child = node->child(entry.name)) {
                return child->hasValue();
            } else {
                return false;
            }
        });
    }

    return result;
}

Blob MaskingFileSystem::_read(const FileSystemPath &path) const {
    assert(!path.isEmpty());
    if (isMasked(path))
        throw FileSystemException(FileSystemException::READ_FAILED_PATH_DOESNT_EXIST, path);
    return _base->read(path);
}

std::unique_ptr<InputStream> MaskingFileSystem::_openForReading(const FileSystemPath &path) const {
    assert(!path.isEmpty());
    if (isMasked(path))
        throw FileSystemException(FileSystemException::READ_FAILED_PATH_DOESNT_EXIST, path);
    return _base->openForReading(path);
}

bool MaskingFileSystem::_remove(const FileSystemPath &path) {
    assert(!path.isEmpty());

    if (isMasked(path))
        return false;

    FileStat stat = _base->stat(path);
    if (!stat)
        return false;

    mask(path);
    return true;
}