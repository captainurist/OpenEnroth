#include "NullFileSystem.h"

#include <memory>
#include <vector>
#include <string>

#include "Library/FileSystem/Interface/FileSystemError.h"

#include "Utility/String/Join.h"

Result<bool> NullFileSystem::_exists(FileSystemPathView path) const {
    return false;
}

Result<FileStat> NullFileSystem::_stat(FileSystemPathView path) const {
    return FileStat();
}

Result<void> NullFileSystem::_ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const {
    if (path.isEmpty()) {
        entries->clear();
        return {};
    }
    return fileSystemError(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);
}

Result<Blob> NullFileSystem::_read(FileSystemPathView path) const {
    return readError(path);
}

Result<std::unique_ptr<InputStream>> NullFileSystem::_openForReading(FileSystemPathView path) const {
    return readError(path);
}

std::string NullFileSystem::_displayPath(FileSystemPathView path) const {
    return join("null://", path.string());
}

Error NullFileSystem::readError(FileSystemPathView path) const {
    return fileSystemError(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
}
