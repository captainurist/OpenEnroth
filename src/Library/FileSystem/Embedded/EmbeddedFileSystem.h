#pragma once

#include <vector>
#include <memory>

#include <cmrc/cmrc.hpp>

#include "Library/FileSystem/Interface/ReadOnlyFileSystem.h"

class EmbeddedFileSystem : public ReadOnlyFileSystem {
 public:
    explicit EmbeddedFileSystem(cmrc::embedded_filesystem base);
    virtual ~EmbeddedFileSystem();

 private:
    virtual bool _exists(const FileSystemPath &path) const override;
    virtual FileStat _stat(const FileSystemPath &path) const override;
    virtual std::vector<DirectoryEntry> _ls(const FileSystemPath &path) const override;
    virtual Blob _read(const FileSystemPath &path) const override;
    virtual std::unique_ptr<InputStream> _openForReading(const FileSystemPath &path) const override;

 private:
    cmrc::embedded_filesystem _base;
};