#pragma once

#include "Utility/Error/Error.h"

#include "FileSystemFwd.h"
#include "FileSystemEnums.h"
#include "FileSystemPathView.h"

/**
 * @param fs                            File system that the error happened in, used to format display paths.
 * @param error                         What went wrong.
 * @param arg0                          Path that the failed operation was called with.
 * @return                              `Error` with a formatted message.
 */
[[nodiscard]] Error fileSystemError(const FileSystem *fs, FileSystemError error, FileSystemPathView arg0);
[[nodiscard]] Error fileSystemError(const FileSystem *fs, FileSystemError error, FileSystemPathView arg0, FileSystemPathView arg1);
