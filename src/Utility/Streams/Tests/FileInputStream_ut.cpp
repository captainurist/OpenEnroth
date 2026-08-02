#include <cstdlib>
#include <string>
#include <filesystem>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/Streams/FileInputStream.h"
#include "Utility/Exception.h"

UNIT_TEST(FileInputStream, Skip) {
    const char *tmpfile = "tmp_test.txt";
    std::string data(3000, 'a');

    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write(data.data(), data.size());
    out.close();

    FileInputStream in(tmpfile);
    size_t bytes = in.skip(50);
    EXPECT_EQ(bytes, 50);

    bytes = in.skip(2000);
    EXPECT_EQ(bytes, 2000);

    char buf[1024] = {};
    bytes = in.read(buf, 1024);
    EXPECT_EQ(bytes, 950);
    EXPECT_EQ(std::string_view(buf, 950), std::string(950, 'a'));
    in.close();
}

UNIT_TEST(FileInputStream, ExceptionMessages) {
    const char *fileName = "afjhrbluxnkskghelxrigjmgdhckeog.txt";

    EXPECT_FALSE(std::filesystem::exists(fileName));
    EXPECT_THROW_MESSAGE(FileInputStream in(fileName), fileName);
}

UNIT_TEST(FileInputStream, ReadUntil) {
    const char *tmpfile = "tmp_readuntil_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    const char data[] = "hello\0world\0!";
    out.write(data, sizeof(data) - 1);
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readUntil('\0'), "hello");
    EXPECT_EQ(in.readUntil('\0'), "world");
    EXPECT_EQ(in.readAll(), "!");
    in.close();
}

UNIT_TEST(FileInputStream, ReadUntilLargeData) {
    const char *tmpfile = "tmp_readuntil_large_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    std::string first(10000, 'a');
    std::string second(5000, 'b');

    FileOutputStream out(tmpfile);
    out.write(first.data(), first.size());
    char delim = '\0';
    out.write(&delim, 1);
    out.write(second.data(), second.size());
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readUntil('\0'), first);
    EXPECT_EQ(in.readAll(), second);
    in.close();
}

UNIT_TEST(FileInputStream, ReadSmallChunks) {
    const char *tmpfile = "tmp_readchunks_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    std::string data(20000, 'x');
    for (size_t i = 0; i < data.size(); i++)
        data[i] = static_cast<char>('a' + (i % 26));

    FileOutputStream out(tmpfile);
    out.write(data.data(), data.size());
    out.close();

    FileInputStream in(tmpfile);
    std::string result;
    char buf[100];
    while (true) {
        size_t bytes = in.read(buf, sizeof(buf));
        if (bytes == 0) break;
        result.append(buf, bytes);
    }
    EXPECT_EQ(result, data);
    in.close();
}

UNIT_TEST(FileInputStream, ReadAllLargeFile) {
    const char *tmpfile = "tmp_readall_large_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    std::string data(50000, 'z');
    FileOutputStream out(tmpfile);
    out.write(data.data(), data.size());
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), data);
    in.close();
}

UNIT_TEST(FileInputStream, SkipAndRead) {
    const char *tmpfile = "tmp_skipread_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    std::string data(20000, 'a');
    for (size_t i = 0; i < data.size(); i++)
        data[i] = static_cast<char>('a' + (i % 26));

    FileOutputStream out(tmpfile);
    out.write(data.data(), data.size());
    out.close();

    FileInputStream in(tmpfile);

    size_t skipped = in.skip(50);
    EXPECT_EQ(skipped, 50);

    skipped = in.skip(15000);
    EXPECT_EQ(skipped, 15000);

    std::string remaining = in.readAll();
    EXPECT_EQ(remaining, data.substr(15050));
    in.close();
}

UNIT_TEST(FileInputStream, LargeSkip) {
    const char *tmpfile = "tmp_largeskip_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    std::string data(2000, 'a');
    for (size_t i = 0; i < data.size(); i++)
        data[i] = static_cast<char>('a' + (i % 26));

    FileOutputStream out(tmpfile);
    out.write(data.data(), data.size());
    out.close();

    // Use a small buffer so that skip(200) exceeds buffer size and triggers the seek path.
    FileInputStream in(tmpfile, 128);

    // Small skip: goes through the buffer.
    size_t skipped = in.skip(50);
    EXPECT_EQ(skipped, 50);

    // Note that this one doesn't seek either - it drains the 78 buffered bytes first, and the remaining 122 bytes
    // still fit into the buffer. See `LargeSkipUsesSeek` for the test that does reach the seeking branch.
    skipped = in.skip(200);
    EXPECT_EQ(skipped, 200);

    std::string remaining = in.readAll();
    EXPECT_EQ(remaining, data.substr(250));
    in.close();
}

UNIT_TEST(FileInputStream, LargeSkipPastEnd) {
    const char *tmpfile = "tmp_largeskip_end_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello");
    out.close();

    FileInputStream in(tmpfile, 4);

    // Skip more than the file contains.
    size_t skipped = in.skip(1000);
    EXPECT_EQ(skipped, 5);

    EXPECT_EQ(in.readAll(), "");
    in.close();
}

UNIT_TEST(FileInputStream, CloseIdempotent) {
    const char *tmpfile = "tmp_closeidem_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello");
    out.close();

    FileInputStream in(tmpfile);
    in.close();
    EXPECT_FALSE(in.isOpen());
    EXPECT_NO_THROW(in.close()); // Double close is fine.
    EXPECT_FALSE(in.isOpen());
}

UNIT_TEST(FileInputStream, ReopenAfterClose) {
    const char *tmpfile1 = "tmp_reopen1_test.txt";
    const char *tmpfile2 = "tmp_reopen2_test.txt";
    ScopedTestFileSlot tmp1(tmpfile1);
    ScopedTestFileSlot tmp2(tmpfile2);

    {
        FileOutputStream out(tmpfile1);
        out.write("first");
        out.close();
    }
    {
        FileOutputStream out(tmpfile2);
        out.write("second");
        out.close();
    }

    FileInputStream in(tmpfile1);
    EXPECT_EQ(in.readAll(), "first");
    in.close();

    in.open(tmpfile2);
    EXPECT_TRUE(in.isOpen());
    EXPECT_EQ(in.readAll(), "second");
    in.close();
}

UNIT_TEST(FileInputStream, ReadUntilMultiRefill) {
    const char *tmpfile = "tmp_readuntil_multirefill_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    // 500 bytes of 'a', then a '\0' delimiter, then 200 bytes of 'b'.
    // With a 64-byte buffer, readUntil needs ~8 refills to find the delimiter.
    std::string first(500, 'a');
    std::string second(200, 'b');

    FileOutputStream out(tmpfile);
    out.write(first.data(), first.size());
    char delim = '\0';
    out.write(&delim, 1);
    out.write(second.data(), second.size());
    out.close();

    FileInputStream in(tmpfile, 64);
    EXPECT_EQ(in.readUntil('\0'), first);
    EXPECT_EQ(in.readAll(), second);
    in.close();
}

UNIT_TEST(FileInputStream, ReadUntilNotFound) {
    const char *tmpfile = "tmp_readuntil_notfound_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    // No delimiter in data — readUntil should return everything.
    std::string data(300, 'x');

    FileOutputStream out(tmpfile);
    out.write(data.data(), data.size());
    out.close();

    FileInputStream in(tmpfile, 64);
    EXPECT_EQ(in.readUntil('\0'), data);
    EXPECT_EQ(in.readAll(), "");
    in.close();
}

UNIT_TEST(FileInputStream, ReadAllEmpty) {
    const char *tmpfile = "tmp_readall_empty_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.close(); // Empty file.

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), "");
    in.close();
}

UNIT_TEST(FileInputStream, SizeMatchesFileSize) {
    const char *tmpfile = "tmp_size_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello world!");
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.size(), 12u);
    in.close();
}

UNIT_TEST(FileInputStream, PositionStartsAtZero) {
    const char *tmpfile = "tmp_pos_start_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello");
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.position(), 0u);
    in.close();
}

UNIT_TEST(FileInputStream, PositionAdvancesOnRead) {
    const char *tmpfile = "tmp_pos_read_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello world");
    out.close();

    FileInputStream in(tmpfile);
    char buf[5];
    in.readOrFail(buf, 5);
    EXPECT_EQ(in.position(), 5u);
    in.close();
}

UNIT_TEST(FileInputStream, PositionAdvancesOnSkip) {
    const char *tmpfile = "tmp_pos_skip_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    std::string data(2000, 'x');
    FileOutputStream out(tmpfile);
    out.write(data);
    out.close();

    // Use small buffer to test both buffered and seeked skips.
    FileInputStream in(tmpfile, 128);

    (void) in.skip(50);
    EXPECT_EQ(in.position(), 50u);

    (void) in.skip(200); // Still goes through the buffer, see the comment in `LargeSkip`.
    EXPECT_EQ(in.position(), 250u);

    EXPECT_EQ(in.readAll(), data.substr(250));
    EXPECT_EQ(in.position(), in.size());
    in.close();
}

UNIT_TEST(FileInputStream, PositionAfterReadAll) {
    const char *tmpfile = "tmp_pos_readall_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello");
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), "hello");
    EXPECT_EQ(in.position(), 5u);
    EXPECT_EQ(in.position(), in.size());
    in.close();
}

UNIT_TEST(FileInputStream, LargeSkipUsesSeek) {
    const char *tmpfile = "tmp_skipseek_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    std::string data(2000, 'a');
    for (size_t i = 0; i < data.size(); i++)
        data[i] = static_cast<char>('a' + (i % 26));

    FileOutputStream out(tmpfile);
    out.write(data);
    out.close();

    // Nothing is buffered yet, so this skip goes straight into the seeking branch of `_underflow`.
    FileInputStream in(tmpfile, 128);
    EXPECT_EQ(in.skip(500), 500u);
    EXPECT_EQ(in.position(), 500u);
    EXPECT_EQ(in.readAll(), data.substr(500));
    in.close();
}

UNIT_TEST(FileInputStream, SkipAfterBufferedReadUsesSeek) {
    const char *tmpfile = "tmp_skipseek2_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    std::string data(2000, 'a');
    for (size_t i = 0; i < data.size(); i++)
        data[i] = static_cast<char>('a' + (i % 26));

    FileOutputStream out(tmpfile);
    out.write(data);
    out.close();

    FileInputStream in(tmpfile, 128);
    char buf[10];
    in.readOrFail(buf, 10);

    // 500 bytes doesn't fit into the buffer even after the buffered tail is drained, so this seeks.
    EXPECT_EQ(in.skip(500), 500u);
    EXPECT_EQ(in.position(), 510u);
    EXPECT_EQ(in.readAll(), data.substr(510));
    in.close();
}

UNIT_TEST(FileInputStream, ReadAtBufferSizeBoundary) {
    const char *tmpfile = "tmp_readboundary_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    std::string data(4096, 'a');
    for (size_t i = 0; i < data.size(); i++)
        data[i] = static_cast<char>('a' + (i % 26));

    FileOutputStream out(tmpfile);
    out.write(data);
    out.close();

    // `_underflow` switches between the buffered and the direct path at exactly `bufferSize`.
    for (size_t readSize : {1023, 1024, 1025}) {
        FileInputStream in(tmpfile, 1024);
        std::string buffer(readSize, '\0');
        EXPECT_EQ(in.read(buffer.data(), readSize), readSize);
        EXPECT_EQ(buffer, data.substr(0, readSize));
        EXPECT_EQ(in.position(), readSize);
        EXPECT_EQ(in.readAll(), data.substr(readSize));
        in.close();
    }
}

UNIT_TEST(FileInputStream, BinaryRoundTripAllByteValues) {
    const char *tmpfile = "tmp_binary_test.bin";
    ScopedTestFileSlot tmp(tmpfile);

    std::string data;
    for (int i = 0; i < 8; i++)
        for (int c = 0; c < 256; c++)
            data += static_cast<char>(c);

    FileOutputStream out(tmpfile);
    out.write(data);
    out.close();

    EXPECT_EQ(std::filesystem::file_size(tmpfile), 2048u);

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.size(), 2048u);
    EXPECT_EQ(in.readAll(), data);
    in.close();
}

UNIT_TEST(FileInputStream, EmptyFileSkipAndRead) {
    const char *tmpfile = "tmp_emptyfile_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.close();

    FileInputStream in(tmpfile, 4);
    EXPECT_EQ(in.size(), 0u);
    EXPECT_EQ(in.skip(1000), 0u);
    EXPECT_EQ(in.position(), 0u);

    char buf[10] = {};
    EXPECT_EQ(in.read(buf, sizeof(buf)), 0u);
    EXPECT_EQ(in.read(buf, sizeof(buf)), 0u);
    EXPECT_EQ(in.readUntil('\n'), "");
    EXPECT_EQ(in.position(), 0u);
    in.close();
}

UNIT_TEST(FileInputStream, ReopenWithSmallerBufferSize) {
    const char *tmpfile1 = "tmp_reopenbuf1_test.txt";
    const char *tmpfile2 = "tmp_reopenbuf2_test.txt";
    ScopedTestFileSlot tmp1(tmpfile1);
    ScopedTestFileSlot tmp2(tmpfile2);

    std::string data(2000, 'q');

    FileOutputStream out1(tmpfile1);
    out1.write("first");
    out1.close();

    FileOutputStream out2(tmpfile2);
    out2.write(data);
    out2.close();

    // The buffer allocated for the first stream must not be reused for the second one.
    FileInputStream in(tmpfile1, 4096);
    EXPECT_EQ(in.readAll(), "first");
    in.close();

    in.open(tmpfile2, 64);
    EXPECT_EQ(in.readAll(), data);
    in.close();
}

UNIT_TEST(FileInputStream, TwoConcurrentReaders) {
    const char *tmpfile = "tmp_concurrent_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello world");
    out.close();

    // Two readers on the same file at the same time. On Windows this only works with a permissive share mode.
    FileInputStream in1(tmpfile);
    FileInputStream in2(tmpfile);
    EXPECT_EQ(in1.readAll(), "hello world");
    EXPECT_EQ(in2.readAll(), "hello world");
    in1.close();
    in2.close();
}

UNIT_TEST(FileInputStream, OpenDirectoryThrows) {
    std::string path = std::filesystem::current_path().generic_string();

    // Opening a directory used to fail at first read on POSIX and at open on Windows. Now it always fails at open.
    EXPECT_THROW(FileInputStream in(path), Exception);
}

UNIT_TEST(FileInputStream, DefaultConstructedThenOpen) {
    const char *tmpfile = "tmp_defaultctor_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello");
    out.close();

    { FileInputStream in; } // Destructing a never-opened stream is fine.

    FileInputStream in;
    EXPECT_FALSE(in.isOpen());
    in.open(tmpfile);
    EXPECT_TRUE(in.isOpen());
    EXPECT_EQ(in.readAll(), "hello");
    in.close();
}

UNIT_TEST(FileInputStream, UnicodePath) {
    std::u8string u8path = u8"файл_input.txt"; // "File" in Russian.
    std::string path = reinterpret_cast<const char *>(u8path.c_str());
    ScopedTestFileSlot tmp(path);

    std::string data(300, 'u');

    FileOutputStream out(path);
    out.write(data);
    out.close();

    FileInputStream in(path);
    EXPECT_EQ(in.size(), data.size());
    EXPECT_EQ(in.readAll(), data);
    in.close();

    std::u8string u8missing = u8"файл_missing_zzz.txt";
    std::string missing = reinterpret_cast<const char *>(u8missing.c_str());
    EXPECT_FALSE(std::filesystem::exists(u8missing));
    EXPECT_THROW_MESSAGE(FileInputStream in2(missing), missing);
}

#ifndef _WINDOWS
UNIT_TEST(FileInputStream, UnsizedStream) {
    // `/dev/null` has no size and reads as empty. It used to open as a zero-sized stream, which happened to look the
    // same - what matters is that `size()` says "unknown" rather than lying about a concrete size.
    FileInputStream in("/dev/null");
    EXPECT_TRUE(in.isOpen());
    EXPECT_EQ(in.size(), static_cast<size_t>(-1));

    // `/dev/null` reads as end-of-stream straight away, so nothing is consumed and `position()` never moves.
    char buf[10] = {};
    EXPECT_EQ(in.position(), 0u);
    EXPECT_EQ(in.read(buf, sizeof(buf)), 0u);
    EXPECT_EQ(in.position(), 0u);
    EXPECT_EQ(in.readAll(), ""); // Goes through the unsized branch of `readAll`.
    EXPECT_EQ(in.position(), 0u);
    EXPECT_EQ(in.skip(1000), 0u); // Read-and-discard, since we can't seek to an unknown end.
    EXPECT_EQ(in.position(), 0u);
    in.close();
}

UNIT_TEST(FileInputStream, UnsizedStreamReadAndSkip) {
    // `/dev/zero` is an endless stream of zero bytes - unsized, but unlike `/dev/null` it actually has data, so it
    // exercises the read-and-discard skip path properly.
    FileInputStream in("/dev/zero", 64);
    EXPECT_EQ(in.size(), static_cast<size_t>(-1));

    char buf[10] = {};
    EXPECT_EQ(in.read(buf, sizeof(buf)), 10u);
    EXPECT_EQ(std::string_view(buf, 10), std::string(10, '\0'));

    EXPECT_EQ(in.skip(5000), 5000u); // Exceeds the buffer, so it goes through the read-and-discard path.
    EXPECT_EQ(in.position(), 5010u);
    in.close();
}
#endif

UNIT_TEST(FileInputStream, PositionResetsOnReopen) {
    const char *tmpfile1 = "tmp_pos_reopen1_test.txt";
    const char *tmpfile2 = "tmp_pos_reopen2_test.txt";
    ScopedTestFileSlot tmp1(tmpfile1);
    ScopedTestFileSlot tmp2(tmpfile2);

    FileOutputStream out1(tmpfile1);
    out1.write("first");
    out1.close();

    FileOutputStream out2(tmpfile2);
    out2.write("second!");
    out2.close();

    FileInputStream in(tmpfile1);
    (void) in.skip(3);
    EXPECT_EQ(in.position(), 3u);
    in.close();

    in.open(tmpfile2);
    EXPECT_EQ(in.position(), 0u);
    EXPECT_EQ(in.size(), 7u);
    in.close();
}

