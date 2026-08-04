#include "DirectoryFileSystem.h"

#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Utility/Error/Result.h"
#include "Library/FileSystem/Interface/FileSystemError.h"
#include "Utility/Streams/FileInputStream.h"
#include "Utility/Streams/FileOutputStream.h"
#include "Utility/UnicodeCrt.h"

DirectoryFileSystem::DirectoryFileSystem(std::string_view root) {
    assert(UnicodeCrt::isInitialized()); // Otherwise std::filesystem will choke on Unicode paths.

    // We need to explicitly check for empty() b/c libstdc++ std::filesystem::absolute chokes on empty path.
    _root = root.empty() ? std::filesystem::current_path() : std::filesystem::absolute(root).lexically_normal();
    _originalRoot = root;
}

DirectoryFileSystem::~DirectoryFileSystem() = default;

Result<bool> DirectoryFileSystem::_exists(FileSystemPathView path) const {
    assert(!path.isEmpty());

    std::error_code ec;
    return std::filesystem::exists(makeBasePath(path), ec); // Returns false on error.
}

Result<FileStat> DirectoryFileSystem::_stat(FileSystemPathView path) const {
    assert(!path.isEmpty());

    std::filesystem::path basePath = makeBasePath(path);

    std::error_code ec;
    std::filesystem::directory_entry entry(basePath, ec);
    bool isRegular = entry.is_regular_file(ec);
    bool isDirectory = !isRegular && entry.is_directory(ec);
    if (!isRegular && !isDirectory)
        return FileStat(); // Return an empty stat on error or if it's not a file / directory.

    std::int64_t size = 0;
    if (isRegular) {
        size = std::filesystem::file_size(basePath, ec);
        if (ec)
            return FileStat();
    }

    FileStat result;
    result.type = isRegular ? FILE_REGULAR : FILE_DIRECTORY;
    result.size = size;
    return result;}

Result<void> DirectoryFileSystem::_ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const {
    std::filesystem::path basePath = makeBasePath(path);

    // Handle the known errors first.
    std::error_code ec;
    std::filesystem::directory_entry parent(basePath, ec);
    bool isParentRegular = parent.is_regular_file(ec);
    bool isParentDirectory = !isParentRegular && parent.is_directory(ec);
    if (path.isEmpty() && !isParentDirectory)
        return {}; // ls("") should always work.
    if (isParentRegular)
        return fileSystemError(this, FS_LS_FAILED_PATH_IS_FILE, path);
    if (!isParentDirectory)
        return fileSystemError(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);

    // Then we do the regular ls and just ignore all errors. The errors we'll get here are most likely
    // permissions-related, and we're ignoring them in stat() and exists().
    for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(makeBasePath(path), ec)) {
        // Unfortunately, std::filesystem is retarded. We can get a directory_entry here for a dir that we don't have
        // permissions for, and which won't be stat-able. Seriously, entry.is_directory() returns true while
        // std::filesystem::exists(entry.path()) just throws. So we need to check for that.
        if (!std::filesystem::exists(entry.path(), ec))
            continue;

        bool isRegular = entry.is_regular_file(ec);
        bool isDirectory = !isRegular && entry.is_directory(ec);
        if (!isRegular && !isDirectory)
            continue;

        std::string name = entry.path().filename().string();
        if (name.find('\\') != std::string::npos)
            continue; // Files with '\\' in filename are not observable through this interface. Don't be a retard.

        DirectoryEntry &resultEntry = entries->emplace_back();
        resultEntry.name = std::move(name);
        resultEntry.type = isRegular ? FILE_REGULAR : FILE_DIRECTORY;
    }
    return {};
}

Result<Blob> DirectoryFileSystem::_read(FileSystemPathView path) const {
    assert(!path.isEmpty());
    return Blob::fromFile(makeBasePath(path).generic_string());
}

Result<void> DirectoryFileSystem::_write(FileSystemPathView path, const Blob &data) {
    assert(!path.isEmpty());
    std::filesystem::path basePath = makeBasePath(path);
    std::filesystem::create_directories(basePath.parent_path());
    FileOutputStream stream;
    co_await stream.open(basePath.generic_string());
    co_await stream.write(data.data(), data.size());
    co_await stream.close();
}

Result<std::unique_ptr<InputStream>> DirectoryFileSystem::_openForReading(FileSystemPathView path) const {
    assert(!path.isEmpty());
    auto stream = std::make_unique<FileInputStream>();
    co_await stream->open(makeBasePath(path).generic_string());
    co_return std::unique_ptr<InputStream>(std::move(stream));
}

Result<std::unique_ptr<OutputStream>> DirectoryFileSystem::_openForWriting(FileSystemPathView path) {
    assert(!path.isEmpty());
    std::filesystem::path basePath = makeBasePath(path);
    std::filesystem::create_directories(basePath.parent_path());
    auto stream = std::make_unique<FileOutputStream>();
    co_await stream->open(basePath.generic_string());
    co_return std::unique_ptr<OutputStream>(std::move(stream));
}

Result<void> DirectoryFileSystem::_rename(FileSystemPathView srcPath, FileSystemPathView dstPath) {
    assert(!srcPath.isEmpty());
    assert(!dstPath.isEmpty());

    std::filesystem::path srcBasePath = makeBasePath(srcPath);
    std::filesystem::path dstBasePath = makeBasePath(dstPath);

    std::error_code ec;
    if (std::filesystem::is_directory(dstBasePath, ec))
        return fileSystemError(this, FS_RENAME_FAILED_DST_IS_DIR, srcPath, dstPath);

    // This call will copy the file if POSIX rename() fails, so if it errors out then we can't do anything either.
    std::filesystem::rename(srcBasePath, dstBasePath, ec);
    if (ec)
        return Error(ec, "Could not rename '{}' to '{}': {}", displayPath(srcPath), displayPath(dstPath), ec.message());
    return {};
}

Result<bool> DirectoryFileSystem::_remove(FileSystemPathView path) {
    assert(!path.isEmpty());
    return std::filesystem::remove_all(makeBasePath(path)) > 0;
}

std::string DirectoryFileSystem::_displayPath(FileSystemPathView path) const {
    return makeBasePath(path).generic_string();
}

std::filesystem::path DirectoryFileSystem::makeBasePath(FileSystemPathView path) const {
    return _root / path.string();
}
