#include <filesystem>
#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Streams/FileHandle.h"
#include "Utility/Exception.h"

static std::string makeData(size_t size) {
    std::string result(size, '\0');
    for (size_t i = 0; i < size; i++)
        result[i] = static_cast<char>('a' + (i % 26));
    return result;
}

static void writeFile(std::string_view path, std::string_view data) {
    FileHandle handle;
    handle.openForWriting(path);
    handle.write(data.data(), data.size());
    EXPECT_EQ(handle.close(), 0);
}

UNIT_TEST(FileHandle, WriteThenReadRoundTrip) {
    const char *tmpfile = "tmp_fh_chunked.bin";
    ScopedTestFileSlot tmp(tmpfile);

    std::string data = makeData(5000);

    {
        FileHandle handle;
        handle.openForWriting(tmpfile);
        handle.write(data.data(), data.size());
        EXPECT_EQ(handle.close(), 0);
    }

    EXPECT_EQ(std::filesystem::file_size(tmpfile), data.size());

    FileHandle handle;
    handle.openForReading(tmpfile);
    EXPECT_EQ(handle.size(), data.size());

    std::string buffer(data.size(), '\0');
    EXPECT_EQ(handle.read(buffer.data(), buffer.size()), data.size());
    EXPECT_EQ(buffer, data);
    EXPECT_EQ(handle.close(), 0);
}

UNIT_TEST(FileHandle, ReadStopsAtEndOfFile) {
    const char *tmpfile = "tmp_fh_eof.bin";
    ScopedTestFileSlot tmp(tmpfile);
    writeFile(tmpfile, "0123456789");

    FileHandle handle;
    handle.openForReading(tmpfile);

    char buffer[100] = {};
    EXPECT_EQ(handle.read(buffer, sizeof(buffer)), 10u);
    EXPECT_EQ(handle.read(buffer, sizeof(buffer)), 0u);
    EXPECT_EQ(handle.close(), 0);
}

UNIT_TEST(FileHandle, Size) {
    const char *tmpfile = "tmp_fh_size.bin";
    ScopedTestFileSlot tmp(tmpfile);
    writeFile(tmpfile, "hello world!");

    FileHandle handle;
    handle.openForReading(tmpfile);
    EXPECT_EQ(handle.size(), 12u);

    char buffer[5] = {};
    EXPECT_EQ(handle.read(buffer, sizeof(buffer)), 5u);
    EXPECT_EQ(handle.size(), 12u); // Reading doesn't move it.
    EXPECT_EQ(handle.close(), 0);
}

UNIT_TEST(FileHandle, SizeIsNotCached) {
    const char *tmpfile = "tmp_fh_livesize.bin";
    ScopedTestFileSlot tmp(tmpfile);
    writeFile(tmpfile, "hello");

    FileHandle handle;
    handle.openForReading(tmpfile);
    EXPECT_EQ(handle.size(), 5u);

    writeFile(tmpfile, "hello world!"); // Rewrite the file behind our back.

    EXPECT_EQ(handle.size(), 12u); // `size()` queries the OS every time, so it sees the new size.
    EXPECT_EQ(handle.close(), 0);
}

UNIT_TEST(FileHandle, Seek) {
    const char *tmpfile = "tmp_fh_seek.bin";
    ScopedTestFileSlot tmp(tmpfile);

    std::string data = makeData(2000);
    writeFile(tmpfile, data);

    FileHandle handle;
    handle.openForReading(tmpfile);

    char buffer[10] = {};
    EXPECT_EQ(handle.read(buffer, sizeof(buffer)), 10u);
    EXPECT_EQ(std::string_view(buffer, 10), data.substr(0, 10));

    handle.seek(500);
    EXPECT_EQ(handle.read(buffer, sizeof(buffer)), 10u);
    EXPECT_EQ(std::string_view(buffer, 10), data.substr(500, 10));
    EXPECT_EQ(handle.close(), 0);
}

UNIT_TEST(FileHandle, OpenForWritingTruncates) {
    const char *tmpfile = "tmp_fh_trunc.bin";
    ScopedTestFileSlot tmp(tmpfile);

    writeFile(tmpfile, makeData(100));
    EXPECT_EQ(std::filesystem::file_size(tmpfile), 100u);

    writeFile(tmpfile, "ab");
    EXPECT_EQ(std::filesystem::file_size(tmpfile), 2u);
}

UNIT_TEST(FileHandle, OpenForWritingCreatesEmptyFile) {
    const char *tmpfile = "tmp_fh_empty.bin";
    ScopedTestFileSlot tmp(tmpfile);

    FileHandle handle;
    handle.openForWriting(tmpfile);
    EXPECT_TRUE(handle.isOpen());
    EXPECT_EQ(handle.close(), 0);

    EXPECT_TRUE(std::filesystem::exists(tmpfile));
    EXPECT_EQ(std::filesystem::file_size(tmpfile), 0u);
}

UNIT_TEST(FileHandle, OpenMissingFileThrows) {
    const char *missing = "aksjdhfoiquwhefiuhqwef.bin";
    EXPECT_FALSE(std::filesystem::exists(missing));

    FileHandle handle;
    EXPECT_THROW_MESSAGE(handle.openForReading(missing), missing);
    EXPECT_FALSE(handle.isOpen());

    // Make sure the OS error text made it into the message, and not just the path.
    try {
        handle.openForReading(missing);
    } catch (const Exception &e) {
        EXPECT_CONTAINS(std::string_view(e.what()), ": ");
    }
}

#ifndef _WINDOWS
UNIT_TEST(FileHandle, NonRegularFileIsUnsized) {
    // `st_size` is 0 for a character device, which would make it look like an empty file. Report it as unsized
    // instead, so that `InputStream` falls back to reading until end of stream.
    FileHandle handle;
    handle.openForReading("/dev/null");
    EXPECT_TRUE(handle.isOpen());
    EXPECT_EQ(handle.size(), static_cast<size_t>(-1));
    EXPECT_EQ(handle.close(), 0);
}
#endif

UNIT_TEST(FileHandle, OpenDirectoryForReadingThrows) {
    std::string path = std::filesystem::current_path().generic_string();

    FileHandle handle;
    EXPECT_THROW(handle.openForReading(path), Exception);
    EXPECT_FALSE(handle.isOpen());
}

UNIT_TEST(FileHandle, OpenDirectoryForWritingThrows) {
    std::string path = std::filesystem::current_path().generic_string();

    FileHandle handle;
    EXPECT_THROW(handle.openForWriting(path), Exception);
    EXPECT_FALSE(handle.isOpen());
}

UNIT_TEST(FileHandle, CloseIsIdempotent) {
    const char *tmpfile = "tmp_fh_close.bin";
    ScopedTestFileSlot tmp(tmpfile);
    writeFile(tmpfile, "hello");

    FileHandle handle;
    EXPECT_FALSE(handle.isOpen());
    EXPECT_EQ(handle.close(), 0); // Closing a default-constructed handle is fine.

    handle.openForReading(tmpfile);
    EXPECT_TRUE(handle.isOpen());
    EXPECT_EQ(handle.close(), 0);
    EXPECT_FALSE(handle.isOpen());
    EXPECT_EQ(handle.close(), 0);
    EXPECT_FALSE(handle.isOpen());
}

#ifdef __linux__
static size_t openHandleCount() {
    size_t result = 0;
    for ([[maybe_unused]] const auto &entry : std::filesystem::directory_iterator("/proc/self/fd"))
        result++;
    return result;
}

UNIT_TEST(FileHandle, NoHandleLeakOnRepeatedOpenClose) {
    const char *tmpfile = "tmp_fh_leak.bin";
    ScopedTestFileSlot tmp(tmpfile);
    writeFile(tmpfile, "hello");

    // Note that we're counting the open descriptors instead of just relying on running out of them - the default
    // `RLIMIT_NOFILE` is high enough that a leaking loop would happily run to completion.
    size_t handlesBefore = openHandleCount();

    char buffer[10] = {};
    for (int i = 0; i < 64; i++) {
        FileHandle handle;
        handle.openForReading(tmpfile);
        EXPECT_EQ(handle.read(buffer, sizeof(buffer)), 5u);
        EXPECT_EQ(handle.close(), 0);
    }

    EXPECT_EQ(openHandleCount(), handlesBefore);
}

UNIT_TEST(FileHandle, DestructorClosesHandle) {
    const char *tmpfile = "tmp_fh_dtor.bin";
    ScopedTestFileSlot tmp(tmpfile);
    writeFile(tmpfile, "hello");

    size_t handlesBefore = openHandleCount();

    for (int i = 0; i < 64; i++) {
        FileHandle handle;
        handle.openForReading(tmpfile); // No explicit close() - the destructor has to release the handle.
    }

    EXPECT_EQ(openHandleCount(), handlesBefore);
}

UNIT_TEST(FileHandle, NoHandleLeakWhenOpenThrows) {
    std::string directory = std::filesystem::current_path().generic_string();

    // `open` succeeds for a directory on POSIX, and it's the `S_ISREG` check that throws - so this exercises the
    // cleanup path that has to release the handle we just opened.
    size_t handlesBefore = openHandleCount();

    for (int i = 0; i < 64; i++) {
        FileHandle handle;
        EXPECT_THROW(handle.openForReading(directory), Exception);
    }

    EXPECT_EQ(openHandleCount(), handlesBefore);
}
#endif

UNIT_TEST(FileHandle, UnicodePath) {
    std::u8string u8path = u8"файл_filehandle.bin"; // "File" in Russian.
    std::string path = reinterpret_cast<const char *>(u8path.c_str());

    ScopedTestFileSlot tmp(path);

    std::string data = makeData(300);
    writeFile(path, data);

    // Check with the UTF-8 api directly, without going through our own classes.
    EXPECT_TRUE(std::filesystem::exists(u8path));
    EXPECT_EQ(std::filesystem::file_size(u8path), data.size());

    FileHandle handle;
    handle.openForReading(path);
    EXPECT_EQ(handle.displayPath(), path);
    std::string buffer(data.size(), '\0');
    EXPECT_EQ(handle.read(buffer.data(), buffer.size()), data.size());
    EXPECT_EQ(buffer, data);
    EXPECT_EQ(handle.close(), 0);
}
