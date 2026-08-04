#include "EmbeddedFileSystem.h"

#include <vector>
#include <string>
#include <memory>

#include "Utility/Streams/MemoryInputStream.h"
#include "Utility/String/Join.h"

EmbeddedFileSystem::EmbeddedFileSystem(cmrc::embedded_filesystem base, std::string_view displayName) : _base(base), _displayName(displayName) {}

EmbeddedFileSystem::~EmbeddedFileSystem() = default;

Result<bool> EmbeddedFileSystem::_exists(FileSystemPathView path) const {
    return _base.exists(std::string(path.string()));
}

Result<FileStat> EmbeddedFileSystem::_stat(FileSystemPathView path) const {
    std::string stringPath(path.string());

    if (!_base.exists(stringPath))
        co_return FileStat();

    if (_base.is_directory(stringPath))
        co_return FileStat(FILE_DIRECTORY, 0);

    cmrc::file file = _base.open(stringPath); // cmrc is external code and can throw - the coroutine catches.
    co_return FileStat(FILE_REGULAR, file.size());
}

Result<void> EmbeddedFileSystem::_ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const {
    // cmrc is external code and can throw - the coroutine catches.
    for (const cmrc::directory_entry &entry : _base.iterate_directory(std::string(path.string())))
        entries->push_back(DirectoryEntry(entry.filename(), entry.is_file() ? FILE_REGULAR : FILE_DIRECTORY));
    co_return; // A function is only a coroutine if it has a co_* keyword in it, and we need the frame here.
}

Result<Blob> EmbeddedFileSystem::_read(FileSystemPathView path) const {
    cmrc::file file = _base.open(std::string(path.string())); // cmrc is external code and can throw - the coroutine catches.
    co_return Blob::view(file.begin(), file.size()).withDisplayPath(displayPath(path));
}

Result<std::unique_ptr<InputStream>> EmbeddedFileSystem::_openForReading(FileSystemPathView path) const {
    cmrc::file file = _base.open(std::string(path.string())); // cmrc is external code and can throw - the coroutine catches.
    co_return std::unique_ptr<InputStream>(std::make_unique<MemoryInputStream>(file.begin(), file.size(), displayPath(path)));
}

std::string EmbeddedFileSystem::_displayPath(FileSystemPathView path) const {
    return join(_displayName, "://", path.string());
}
