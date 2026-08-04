#include "ScopedTestFile.h"

#include <filesystem>

#include "Utility/Streams/FileOutputStream.h"

ScopedTestFile::ScopedTestFile(std::string_view path, std::string_view contents) : _path(path) {
    FileOutputStream stream;
    stream.open(_path).orThrow(); // Throwing is the error handling in tests.
    stream.write(contents).orThrow();
    stream.close().orThrow();
}

ScopedTestFile::~ScopedTestFile() {
    std::error_code ec;
    std::filesystem::remove(_path, ec);
}
