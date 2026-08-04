#pragma once

#include <memory>
#include <string>
#include <vector>

#include <cmrc/cmrc.hpp>

#include "Utility/Error/Result.h"
#include "Library/FileSystem/Interface/ReadOnlyFileSystem.h"

class EmbeddedFileSystem : public ReadOnlyFileSystem {
 public:
    explicit EmbeddedFileSystem(cmrc::embedded_filesystem base, std::string_view displayName);
    virtual ~EmbeddedFileSystem();

 private:
    virtual Result<bool> _exists(FileSystemPathView path) const override;
    virtual Result<FileStat> _stat(FileSystemPathView path) const override;
    virtual Result<void> _ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const override;
    virtual Result<Blob> _read(FileSystemPathView path) const override;
    virtual Result<std::unique_ptr<InputStream>> _openForReading(FileSystemPathView path) const override;
    virtual std::string _displayPath(FileSystemPathView path) const override;

 private:
    cmrc::embedded_filesystem _base;
    std::string _displayName;
};
