#include "ReadOnlyFileSystem.h"

#include <memory> // NOLINT: Linter going insane here for some reason.

#include "FileSystemError.h"

Result<void> ReadOnlyFileSystem::_write(FileSystemPathView path, const Blob &data) {
    return writeError(path);
}

Result<std::unique_ptr<OutputStream>> ReadOnlyFileSystem::_openForWriting(FileSystemPathView path) {
    return writeError(path);
}

Result<void> ReadOnlyFileSystem::_rename(FileSystemPathView srcPath, FileSystemPathView dstPath) {
    return fileSystemError(this, FS_RENAME_FAILED_DST_NOT_WRITEABLE, srcPath, dstPath);
}

Result<bool> ReadOnlyFileSystem::_remove(FileSystemPathView path) {
    if (!co_await _exists(path))
        co_return false;

    co_return fileSystemError(this, FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, path);
}

Error ReadOnlyFileSystem::writeError(FileSystemPathView path) const {
    return fileSystemError(this, FS_WRITE_FAILED_PATH_NOT_WRITEABLE, path);
}
