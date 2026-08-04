#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Utility/Error/Result.h"
#include "Library/FileSystem/Interface/FileSystem.h"

class ProxyFileSystem : public FileSystem {
 public:
    explicit ProxyFileSystem(FileSystem *base = nullptr): _base(base) {}

    FileSystem *base() const {
        return _base;
    }

    void setBase(FileSystem *base) {
        _base = base;
    }

 protected:
    virtual Result<bool> _exists(FileSystemPathView path) const override;
    virtual Result<FileStat> _stat(FileSystemPathView path) const override;
    virtual Result<void> _ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const override;
    virtual Result<Blob> _read(FileSystemPathView path) const override;
    virtual Result<void> _write(FileSystemPathView path, const Blob &data) override;
    virtual Result<std::unique_ptr<InputStream>> _openForReading(FileSystemPathView path) const override;
    virtual Result<std::unique_ptr<OutputStream>> _openForWriting(FileSystemPathView path) override;
    virtual Result<void> _rename(FileSystemPathView srcPath, FileSystemPathView dstPath) override;
    virtual Result<bool> _remove(FileSystemPathView path) override;
    virtual std::string _displayPath(FileSystemPathView path) const override;

    FileSystem *nonNullBase() const;

 private:
    FileSystem *_base = nullptr;
};
