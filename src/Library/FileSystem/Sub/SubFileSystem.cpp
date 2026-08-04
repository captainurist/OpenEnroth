#include "SubFileSystem.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "Utility/Error/Result.h"

SubFileSystem::SubFileSystem(FileSystemPathView basePath, FileSystem *base)
    : _base(base), _basePath(basePath) {
    assert(_base);
}

SubFileSystem::SubFileSystem(std::string_view basePath, FileSystem *base)
    : SubFileSystem(FileSystemPathView(FileSystemPath(basePath)), base) {
}

Result<bool> SubFileSystem::_exists(FileSystemPathView path) const {
    return _base->exists(_basePath / path);
}

Result<FileStat> SubFileSystem::_stat(FileSystemPathView path) const {
    return _base->stat(_basePath / path);
}

Result<void> SubFileSystem::_ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const {
    return _base->ls(_basePath / path, entries);
}

Result<Blob> SubFileSystem::_read(FileSystemPathView path) const {
    return _base->read(_basePath / path);
}

Result<void> SubFileSystem::_write(FileSystemPathView path, const Blob &data) {
    return _base->write(_basePath / path, data);
}

Result<std::unique_ptr<InputStream>> SubFileSystem::_openForReading(FileSystemPathView path) const {
    return _base->openForReading(_basePath / path);
}

Result<std::unique_ptr<OutputStream>> SubFileSystem::_openForWriting(FileSystemPathView path) {
    return _base->openForWriting(_basePath / path);
}

Result<void> SubFileSystem::_rename(FileSystemPathView srcPath, FileSystemPathView dstPath) {
    return _base->rename(_basePath / srcPath, _basePath / dstPath);
}

Result<bool> SubFileSystem::_remove(FileSystemPathView path) {
    return _base->remove(_basePath / path);
}

std::string SubFileSystem::_displayPath(FileSystemPathView path) const {
    return _base->displayPath(_basePath / path);
}
