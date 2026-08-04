#pragma once

#include <memory>

#include "Utility/Error/Result.h"
#include "FileSystem.h"

/**
 * Base class for read-only file systems.
 */
class ReadOnlyFileSystem : public FileSystem {
 private:
    virtual Result<void> _write(FileSystemPathView path, const Blob &data) override;
    virtual Result<std::unique_ptr<OutputStream>> _openForWriting(FileSystemPathView path) override;
    virtual Result<void> _rename(FileSystemPathView srcPath, FileSystemPathView dstPath) override;
    virtual Result<bool> _remove(FileSystemPathView path) override;

    [[nodiscard]] Error writeError(FileSystemPathView path) const;
};
