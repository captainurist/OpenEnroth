#include "LowercaseFileSystem.h"

#include <cassert>
#include <memory>
#include <ranges>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "Utility/Error/Result.h"
#include "Library/FileSystem/Interface/FileSystemError.h"
#include "Library/FileSystem/Proxy/ProxyFileSystem.h"
#include "Utility/String/Ascii.h"
#include "Utility/MapAccess.h"
#include "Utility/Exception.h"

static bool hasUpper(std::string_view s) {
    return std::ranges::any_of(s, &ascii::isUpper);
}

LowercaseFileSystem::LowercaseFileSystem(FileSystem *base): _base(base) {
    assert(_base);
    refresh();
}

LowercaseFileSystem::~LowercaseFileSystem() = default;

void LowercaseFileSystem::refresh() {
    _trie.clear();
    _trie.insertOrAssign({}, detail::LowercaseFileData(FILE_DIRECTORY, ""));
}

Result<bool> LowercaseFileSystem::_exists(FileSystemPathView path) const {
    const auto [basePath, node, tail] = co_await walk(path);
    co_return tail.isEmpty();
}

Result<FileStat> LowercaseFileSystem::_stat(FileSystemPathView path) const {
    const auto [basePath, node, tail] = co_await walk(path);
    if (!tail.isEmpty())
        co_return FileStat();
    if (node->value().conflicting)
        co_return FileStat(FILE_REGULAR, 0); // Conflicts are reported as empty files.
    co_return _base->stat(basePath);
}

Result<void> LowercaseFileSystem::_ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const {
    const auto [basePath, node, tail] = co_await walk(path);
    if (!tail.isEmpty())
        co_await fileSystemError(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);
    if (node->value().type != FILE_DIRECTORY)
        co_await fileSystemError(this, FS_LS_FAILED_PATH_IS_FILE, path);

    co_await cacheLs(node, basePath);

    for (const auto &[name, child] : node->children())
        entries->push_back(DirectoryEntry(name, child->value().type));
}

Result<Blob> LowercaseFileSystem::_read(FileSystemPathView path) const {
    co_return _base->read(co_await locateForReading(path));
}

Result<void> LowercaseFileSystem::_write(FileSystemPathView path, const Blob &data) {
    const auto &[basePath, node, tail] = co_await locateForWriting(path);
    co_await _base->write(basePath, data);
    cacheInsert(node, tail, FILE_REGULAR);
}

Result<std::unique_ptr<InputStream>> LowercaseFileSystem::_openForReading(FileSystemPathView path) const {
    co_return _base->openForReading(co_await locateForReading(path));
}

Result<std::unique_ptr<OutputStream>> LowercaseFileSystem::_openForWriting(FileSystemPathView path) {
    const auto &[basePath, node, tail] = co_await locateForWriting(path);
    std::unique_ptr<OutputStream> result = co_await _base->openForWriting(basePath);
    cacheInsert(node, tail, FILE_REGULAR);
    co_return result;
}

Result<void> LowercaseFileSystem::_rename(FileSystemPathView srcPath, FileSystemPathView dstPath) {
    if (hasUpper(dstPath.string()))
        co_await fileSystemError(this, FS_RENAME_FAILED_DST_NOT_WRITEABLE, srcPath, dstPath);

    auto [srcBasePath, srcNode, srcTail] = co_await walk(srcPath);
    if (!srcTail.isEmpty())
        co_await fileSystemError(this, FS_RENAME_FAILED_SRC_DOESNT_EXIST, srcPath, dstPath);
    if (srcNode->value().conflicting)
        co_await fileSystemError(this, FS_RENAME_FAILED_SRC_NOT_WRITEABLE, srcPath, dstPath);

    auto [dstBasePath, dstNode, dstTail] = co_await walk(dstPath);
    if (dstNode->value().type == FILE_DIRECTORY && dstTail.isEmpty())
        co_await fileSystemError(this, FS_RENAME_FAILED_DST_IS_DIR, srcPath, dstPath);
    if (srcNode->value().type == FILE_DIRECTORY && dstTail.isEmpty())
        co_await fileSystemError(this, FS_RENAME_FAILED_SRC_IS_DIR_DST_IS_FILE, srcPath, dstPath);
    if (dstNode->value().conflicting)
        co_await fileSystemError(this, FS_RENAME_FAILED_DST_NOT_WRITEABLE, srcPath, dstPath);

    dstBasePath /= dstTail;
    if (Result<void> renamed = _base->rename(srcBasePath, dstBasePath); !renamed) {
        // We have no idea about the state of the underlying FS now. Don't bother checking, just invalidate the caches.
        invalidateLs(srcNode->parent());
        invalidateLs(dstTail.isEmpty() ? dstNode->parent() : dstNode);
        co_await std::move(renamed); // Always exits - `renamed` holds an error here.
    }

    cacheInsert(dstNode, dstTail, srcNode->value().type);
    cacheRemove(srcNode);
}

Result<bool> LowercaseFileSystem::_remove(FileSystemPathView path) {
    assert(!path.isEmpty());

    auto [basePath, node, tail] = co_await walk(path);
    if (!tail.isEmpty())
        co_return false;

    if (node->value().conflicting)
        co_await fileSystemError(this, FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, path);

    // Return value of remove() doesn't matter here, from this file system's pov we are deleting an existing entry.
    if (Result<bool> removed = _base->remove(basePath); !removed.ok()) {
        // An error should mean that the file/folder wasn't removed. However, if it's a folder then some of the files
        // might have been removed, so we need to invalidate the caches in this case.
        if (node->value().type == FILE_DIRECTORY)
            invalidateLs(node);
        co_return std::move(removed).error();
    }

    cacheRemove(node);
    co_return true;
}

std::string LowercaseFileSystem::_displayPath(FileSystemPathView path) const {
    Result<std::tuple<FileSystemPath, Node *, FileSystemPathView>> walked = walk(path);
    if (!walked)
        return _base->displayPath(path); // Best effort - display paths must always be produced.
    auto &[basePath, node, tail] = *walked;
    return _base->displayPath(basePath / tail);
}

Result<std::tuple<FileSystemPath, LowercaseFileSystem::Node *, FileSystemPathView>> LowercaseFileSystem::walk(FileSystemPathView path) const {
    Node *node = _trie.root();
    if (path.isEmpty())
        co_return std::tuple(FileSystemPath(), node, FileSystemPathView());

    FileSystemPath basePath;
    for (std::string_view chunk : path.split()) {
        if (node->value().type != FILE_DIRECTORY)
            co_return std::tuple(std::move(basePath), node, path.split().tailAt(chunk));

        co_await cacheLs(node, basePath);

        Node *child = node->child(chunk);
        if (!child)
            co_return std::tuple(std::move(basePath), node, path.split().tailAt(chunk));

        node = child;
        basePath /= child->value().baseName;
    }

    co_return std::tuple(std::move(basePath), node, FileSystemPathView());
}

Result<void> LowercaseFileSystem::cacheLs(Node *node, FileSystemPathView basePath) const {
    assert(node->value().type == FILE_DIRECTORY);

    if (node->value().listed)
        co_return;

    std::vector<DirectoryEntry> entries = co_await _base->ls(basePath);
    for (DirectoryEntry &entry : entries) {
        std::string lowerEntryName = ascii::toLower(entry.name);

        auto pos = node->children().find(lowerEntryName);
        if (pos != node->children().end()) {
            pos->second->value().type = FILE_REGULAR;
            pos->second->value().conflicting = true;
            continue;
        }

        _trie.insertOrAssign(node,
                             FileSystemPathView::fromNormalized(lowerEntryName),
                             detail::LowercaseFileData(entry.type, std::move(entry.name)));
    }

    node->value().listed = true;
}

void LowercaseFileSystem::invalidateLs(Node *node) const {
    assert(node->value().type == FILE_DIRECTORY);

    node->value().listed = false;
    _trie.chop(node);
}

void LowercaseFileSystem::cacheRemove(Node *node) const {
    Node *prev = node;
    Node *next = node->parent();

    while (next->children().size() == 1 && next != _trie.root()) {
        prev = next;
        next = next->parent();
    }

    if (prev == node) {
        _trie.erase(node);
    } else {
        // We don't know if the underlying FS keeps empty folders or not, so we just invalidate the caches. We might drop
        // more than we really should, but the alternative approach here is to call ProxyFileSystem::exists, and we need
        // to construct a base path for that... just not worth it.
        invalidateLs(next);
    }
}

void LowercaseFileSystem::cacheInsert(Node *node, FileSystemPathView tail, FileType type) const {
    if (tail.isEmpty())
        return;

    assert(node->value().type == FILE_DIRECTORY);

    auto chunks = tail.split();
    auto pos = chunks.begin();
    auto end = chunks.end();

    std::string_view firstChunk = *pos;
    ++pos;
    assert(!node->children().contains(firstChunk));

    FileType nodeType = pos == end ? type : FILE_DIRECTORY;
    _trie.insertOrAssign(node,
                         FileSystemPathView::fromNormalized(firstChunk),
                         detail::LowercaseFileData(nodeType, std::string(firstChunk)));
}

Result<FileSystemPath> LowercaseFileSystem::locateForReading(FileSystemPathView path) const {
    auto [basePath, node, tail] = co_await walk(path);
    if (!tail.isEmpty())
        co_await fileSystemError(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
    if (node->value().type == FILE_DIRECTORY)
        co_await fileSystemError(this, FS_READ_FAILED_PATH_IS_DIR, path);
    if (node->value().conflicting)
        co_await fileSystemError(this, FS_READ_FAILED_PATH_NOT_READABLE, path);
    co_return std::move(basePath);
}

Result<std::tuple<FileSystemPath, LowercaseFileSystem::Node *, FileSystemPathView>> LowercaseFileSystem::locateForWriting(FileSystemPathView path) {
    if (hasUpper(path.string()))
        co_await fileSystemError(this, FS_WRITE_FAILED_PATH_NOT_WRITEABLE, path);

    auto result = co_await walk(path);
    auto &[basePath, node, tail] = result;

    if (tail.isEmpty() && node->value().type == FILE_DIRECTORY)
        co_await fileSystemError(this, FS_WRITE_FAILED_PATH_IS_DIR, path);
    if (!tail.isEmpty() && node->value().type == FILE_REGULAR)
        co_await fileSystemError(this, FS_WRITE_FAILED_FILE_IN_PATH, path);
    if (node->value().conflicting)
        co_await fileSystemError(this, FS_WRITE_FAILED_PATH_NOT_WRITEABLE, path);

    basePath /= tail;
    co_return result;
}
