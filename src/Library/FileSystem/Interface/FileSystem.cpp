#include "FileSystem.h"

#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Utility/Error/Result.h"
#include "FileSystemPath.h"
#include "FileSystemError.h"

static constexpr size_t COPY_BUFFER_SIZE = 1024 * 1024;

Result<bool> FileSystem::exists(std::string_view path) const {
    return exists(FileSystemPath(path));
}

Result<bool> FileSystem::exists(FileSystemPathView path) const {
    if (path.isEmpty())
        return true; // Root always exists.
    if (path.isEscaping())
        return false; // Escaping paths are not accessible through this interface.
    return _exists(path);
}

Result<FileStat> FileSystem::stat(std::string_view path) const {
    return stat(FileSystemPath(path));
}

Result<FileStat> FileSystem::stat(FileSystemPathView path) const {
    if (path.isEmpty())
        return FileStat(FILE_DIRECTORY, 0);
    if (path.isEscaping())
        return FileStat();
    return _stat(path);
}

Result<std::vector<DirectoryEntry>> FileSystem::ls(std::string_view path) const {
    return ls(FileSystemPath(path));
}

Result<std::vector<DirectoryEntry>> FileSystem::ls(FileSystemPathView path) const {
    if (path.isEscaping())
        return fileSystemError(this, FS_LS_FAILED_PATH_NOT_ACCESSIBLE, path);
    std::vector<DirectoryEntry> result;
    if (Result<void> status = _ls(path, &result); !status)
        return std::move(status).error();
    return result;
}

Result<void> FileSystem::ls(std::string_view path, std::vector<DirectoryEntry> *entries) const {
    return ls(FileSystemPath(path), entries);
}

Result<void> FileSystem::ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const {
    if (path.isEscaping())
        return fileSystemError(this, FS_LS_FAILED_PATH_NOT_ACCESSIBLE, path);
    entries->clear();
    return _ls(path, entries);
}

Result<Blob> FileSystem::read(std::string_view path) const {
    return read(FileSystemPath(path));
}

Result<Blob> FileSystem::read(FileSystemPathView path) const {
    if (path.isEmpty())
        return fileSystemError(this, FS_READ_FAILED_PATH_IS_DIR, path);
    if (path.isEscaping())
        return fileSystemError(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _read(path);
}

Result<void> FileSystem::write(std::string_view path, const Blob &data) {
    return write(FileSystemPath(path), data);
}

Result<void> FileSystem::write(FileSystemPathView path, const Blob &data) {
    if (path.isEmpty())
        return fileSystemError(this, FS_WRITE_FAILED_PATH_IS_DIR, path);
    if (path.isEscaping())
        return fileSystemError(this, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _write(path, data);
}

Result<std::unique_ptr<InputStream>> FileSystem::openForReading(std::string_view path) const {
    return openForReading(FileSystemPath(path));
}

Result<std::unique_ptr<InputStream>> FileSystem::openForReading(FileSystemPathView path) const {
    if (path.isEmpty())
        return fileSystemError(this, FS_READ_FAILED_PATH_IS_DIR, path);
    if (path.isEscaping())
        return fileSystemError(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _openForReading(path);
}

Result<std::unique_ptr<OutputStream>> FileSystem::openForWriting(std::string_view path) {
    return openForWriting(FileSystemPath(path));
}

Result<std::unique_ptr<OutputStream>> FileSystem::openForWriting(FileSystemPathView path) {
    if (path.isEmpty())
        return fileSystemError(this, FS_WRITE_FAILED_PATH_IS_DIR, path);
    if (path.isEscaping())
        return fileSystemError(this, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _openForWriting(path);
}

Result<void> FileSystem::rename(std::string_view srcPath, std::string_view dstPath) {
    return rename(FileSystemPath(srcPath), FileSystemPath(dstPath));
}

Result<void> FileSystem::rename(FileSystemPathView srcPath, FileSystemPathView dstPath) {
    if (srcPath.isEmpty())
        return fileSystemError(this, FS_RENAME_FAILED_SRC_NOT_WRITEABLE, srcPath, dstPath);
    if (srcPath.isEscaping())
        return fileSystemError(this, FS_RENAME_FAILED_SRC_NOT_ACCESSIBLE, srcPath, dstPath);
    if (dstPath.isEmpty())
        return fileSystemError(this, FS_RENAME_FAILED_DST_NOT_WRITEABLE, srcPath, dstPath);
    if (dstPath.isEscaping())
        return fileSystemError(this, FS_RENAME_FAILED_DST_NOT_ACCESSIBLE, srcPath, dstPath);
    if (srcPath == dstPath)
        return {};
    if (srcPath.isPrefixOf(dstPath))
        return fileSystemError(this, FS_RENAME_FAILED_SRC_IS_PARENT_OF_DST, srcPath, dstPath);
    return _rename(srcPath, dstPath);
}

Result<bool> FileSystem::remove(std::string_view path) {
    return remove(FileSystemPath(path));
}

Result<bool> FileSystem::remove(FileSystemPathView path) {
    if (path.isEmpty())
        return fileSystemError(this, FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, path);
    if (path.isEscaping())
        return fileSystemError(this, FS_REMOVE_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _remove(path);
}

std::string FileSystem::displayPath(std::string_view path) const {
    return displayPath(FileSystemPath(path));
}

std::string FileSystem::displayPath(FileSystemPathView path) const {
    return _displayPath(path);
}

Result<void> FileSystem::_rename(FileSystemPathView srcPath, FileSystemPathView dstPath) {
    assert(!srcPath.isEmpty());
    assert(!dstPath.isEmpty());

    FileStat srcStat = co_await stat(srcPath);
    if (!srcStat)
        co_await fileSystemError(this, FS_RENAME_FAILED_SRC_DOESNT_EXIST, srcPath, dstPath);

    FileStat dstStat = co_await stat(dstPath);
    if (dstStat.type == FILE_DIRECTORY)
        co_await fileSystemError(this, FS_RENAME_FAILED_DST_IS_DIR, srcPath, dstPath);
    if (dstStat.type == FILE_REGULAR && srcStat.type == FILE_DIRECTORY)
        co_await fileSystemError(this, FS_RENAME_FAILED_SRC_IS_DIR_DST_IS_FILE, srcPath, dstPath);
    if (dstStat)
        co_await remove(dstPath);

    std::unique_ptr<char[]> buffer;
    auto copyFile = [this, &buffer](FileSystemPathView srcPath, FileSystemPathView dstPath) -> Result<void> {
        std::unique_ptr<InputStream> input = co_await openForReading(srcPath);
        std::unique_ptr<OutputStream> output = co_await openForWriting(dstPath);

        if (!buffer)
            buffer = std::make_unique_for_overwrite<char[]>(COPY_BUFFER_SIZE);

        while (true) {
            size_t bytes = co_await input->read(buffer.get(), COPY_BUFFER_SIZE);
            if (!bytes)
                break;
            co_await output->write(buffer.get(), bytes);
        }
        co_return;
    };

    auto copyDir = [this] (FileSystemPathView srcPath, FileSystemPathView dstPath, const auto &copyAny) -> Result<void> {
        for (const DirectoryEntry &entry : co_await ls(srcPath))
            co_await copyAny(entry.type, srcPath / entry.name, dstPath / entry.name, copyAny);
        co_return;
    };

    auto copyAny = [this, &copyFile, &copyDir] (FileType type, FileSystemPathView srcPath, FileSystemPathView dstPath, const auto &copyAny) -> Result<void> {
        if (type == FILE_REGULAR) {
            co_await copyFile(srcPath, dstPath);
        } else {
            assert(type == FILE_DIRECTORY);
            co_await copyDir(srcPath, dstPath, copyAny);
        }
        co_return;
    };

    co_await copyAny(srcStat.type, srcPath, dstPath, copyAny);
    co_await remove(srcPath);
}
