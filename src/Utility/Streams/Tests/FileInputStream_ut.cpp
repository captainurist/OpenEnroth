#include <cstdlib>
#include <string>
#include <filesystem>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/Streams/FileInputStream.h"

UNIT_TEST(FileInputStream, Skip) {
    const char *tmpfile = "tmp_test.txt";
    std::string data(3000, 'a');

    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write(data.data(), data.size()));
    EXPECT_TRUE(out.close());

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile));
    size_t bytes = in.skip(50).orThrow();
    EXPECT_EQ(bytes, 50);

    bytes = in.skip(2000).orThrow();
    EXPECT_EQ(bytes, 2000);

    char buf[1024] = {};
    bytes = in.read(buf, 1024).orThrow();
    EXPECT_EQ(bytes, 950);
    EXPECT_EQ(std::string_view(buf, 950), std::string(950, 'a'));
    EXPECT_TRUE(in.close());
}

UNIT_TEST(FileInputStream, ExceptionMessages) {
    const char *fileName = "afjhrbluxnkskghelxrigjmgdhckeog.txt";

    EXPECT_FALSE(std::filesystem::exists(fileName));

    FileInputStream in;
    Result<void> opened = in.open(fileName);
    ASSERT_FALSE(opened);
    EXPECT_THAT(opened.error().message(), testing::HasSubstr(fileName));
}

UNIT_TEST(FileInputStream, ReadUntil) {
    const char *tmpfile = "tmp_readuntil_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    const char data[] = "hello\0world\0!";
    EXPECT_TRUE(out.write(data, sizeof(data) - 1));
    EXPECT_TRUE(out.close());

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile));
    EXPECT_EQ(in.readUntil('\0').orThrow(), "hello");
    EXPECT_EQ(in.readUntil('\0').orThrow(), "world");
    EXPECT_EQ(in.readAll().orThrow(), "!");
    EXPECT_TRUE(in.close());
}

UNIT_TEST(FileInputStream, ReadUntilLargeData) {
    const char *tmpfile = "tmp_readuntil_large_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    std::string first(10000, 'a');
    std::string second(5000, 'b');

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write(first.data(), first.size()));
    char delim = '\0';
    EXPECT_TRUE(out.write(&delim, 1));
    EXPECT_TRUE(out.write(second.data(), second.size()));
    EXPECT_TRUE(out.close());

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile));
    EXPECT_EQ(in.readUntil('\0').orThrow(), first);
    EXPECT_EQ(in.readAll().orThrow(), second);
    EXPECT_TRUE(in.close());
}

UNIT_TEST(FileInputStream, ReadSmallChunks) {
    const char *tmpfile = "tmp_readchunks_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    std::string data(20000, 'x');
    for (size_t i = 0; i < data.size(); i++)
        data[i] = static_cast<char>('a' + (i % 26));

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write(data.data(), data.size()));
    EXPECT_TRUE(out.close());

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile));
    std::string result;
    char buf[100];
    while (true) {
        size_t bytes = in.read(buf, sizeof(buf)).orThrow();
        if (bytes == 0) break;
        result.append(buf, bytes);
    }
    EXPECT_EQ(result, data);
    EXPECT_TRUE(in.close());
}

UNIT_TEST(FileInputStream, ReadAllLargeFile) {
    const char *tmpfile = "tmp_readall_large_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    std::string data(50000, 'z');
    FileOutputStream out;
    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write(data.data(), data.size()));
    EXPECT_TRUE(out.close());

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile));
    EXPECT_EQ(in.readAll().orThrow(), data);
    EXPECT_TRUE(in.close());
}

UNIT_TEST(FileInputStream, SkipAndRead) {
    const char *tmpfile = "tmp_skipread_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    std::string data(20000, 'a');
    for (size_t i = 0; i < data.size(); i++)
        data[i] = static_cast<char>('a' + (i % 26));

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write(data.data(), data.size()));
    EXPECT_TRUE(out.close());

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile));

    size_t skipped = in.skip(50).orThrow();
    EXPECT_EQ(skipped, 50);

    skipped = in.skip(15000).orThrow();
    EXPECT_EQ(skipped, 15000);

    std::string remaining = in.readAll().orThrow();
    EXPECT_EQ(remaining, data.substr(15050));
    EXPECT_TRUE(in.close());
}

UNIT_TEST(FileInputStream, LargeSkip) {
    const char *tmpfile = "tmp_largeskip_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    std::string data(2000, 'a');
    for (size_t i = 0; i < data.size(); i++)
        data[i] = static_cast<char>('a' + (i % 26));

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write(data.data(), data.size()));
    EXPECT_TRUE(out.close());

    // Use a small buffer so that skip(200) exceeds buffer size and triggers the seek path.
    FileInputStream in;
    ASSERT_TRUE(in.open(tmpfile, 128));

    // Small skip: goes through buffered fread.
    size_t skipped = in.skip(50).orThrow();
    EXPECT_EQ(skipped, 50);

    // Large skip: exceeds buffer size, triggers fseeko.
    skipped = in.skip(200).orThrow();
    EXPECT_EQ(skipped, 200);

    std::string remaining = in.readAll().orThrow();
    EXPECT_EQ(remaining, data.substr(250));
    EXPECT_TRUE(in.close());
}

UNIT_TEST(FileInputStream, LargeSkipPastEnd) {
    const char *tmpfile = "tmp_largeskip_end_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write("hello"));
    EXPECT_TRUE(out.close());

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile, 4));

    // Skip more than the file contains.
    size_t skipped = in.skip(1000).orThrow();
    EXPECT_EQ(skipped, 5);

    EXPECT_EQ(in.readAll().orThrow(), "");
    EXPECT_TRUE(in.close());
}

UNIT_TEST(FileInputStream, CloseIdempotent) {
    const char *tmpfile = "tmp_closeidem_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write("hello"));
    EXPECT_TRUE(out.close());

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile));
    EXPECT_TRUE(in.close());
    EXPECT_FALSE(in.isOpen());
    EXPECT_TRUE(in.close()); // Double close is fine.
    EXPECT_FALSE(in.isOpen());
}

UNIT_TEST(FileInputStream, ReopenAfterClose) {
    const char *tmpfile1 = "tmp_reopen1_test.txt";
    const char *tmpfile2 = "tmp_reopen2_test.txt";
    ScopedTestFileSlot tmp1(tmpfile1);
    ScopedTestFileSlot tmp2(tmpfile2);

    {
        FileOutputStream out;
        ASSERT_TRUE(out.open(tmpfile1));
        EXPECT_TRUE(out.write("first"));
        EXPECT_TRUE(out.close());
    }
    {
        FileOutputStream out;
        ASSERT_TRUE(out.open(tmpfile2));
        EXPECT_TRUE(out.write("second"));
        EXPECT_TRUE(out.close());
    }

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile1));
    EXPECT_EQ(in.readAll().orThrow(), "first");
    EXPECT_TRUE(in.close());

    EXPECT_TRUE(in.open(tmpfile2));
    EXPECT_TRUE(in.isOpen());
    EXPECT_EQ(in.readAll().orThrow(), "second");
    EXPECT_TRUE(in.close());
}

UNIT_TEST(FileInputStream, ReadUntilMultiRefill) {
    const char *tmpfile = "tmp_readuntil_multirefill_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    // 500 bytes of 'a', then a '\0' delimiter, then 200 bytes of 'b'.
    // With a 64-byte buffer, readUntil needs ~8 refills to find the delimiter.
    std::string first(500, 'a');
    std::string second(200, 'b');

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write(first.data(), first.size()));
    char delim = '\0';
    EXPECT_TRUE(out.write(&delim, 1));
    EXPECT_TRUE(out.write(second.data(), second.size()));
    EXPECT_TRUE(out.close());

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile, 64));
    EXPECT_EQ(in.readUntil('\0').orThrow(), first);
    EXPECT_EQ(in.readAll().orThrow(), second);
    EXPECT_TRUE(in.close());
}

UNIT_TEST(FileInputStream, ReadUntilNotFound) {
    const char *tmpfile = "tmp_readuntil_notfound_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    // No delimiter in data — readUntil should return everything.
    std::string data(300, 'x');

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write(data.data(), data.size()));
    EXPECT_TRUE(out.close());

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile, 64));
    EXPECT_EQ(in.readUntil('\0').orThrow(), data);
    EXPECT_EQ(in.readAll().orThrow(), "");
    EXPECT_TRUE(in.close());
}

UNIT_TEST(FileInputStream, ReadAllEmpty) {
    const char *tmpfile = "tmp_readall_empty_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.close()); // Empty file.

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile));
    EXPECT_EQ(in.readAll().orThrow(), "");
    EXPECT_TRUE(in.close());
}

UNIT_TEST(FileInputStream, SizeMatchesFileSize) {
    const char *tmpfile = "tmp_size_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write("hello world!"));
    EXPECT_TRUE(out.close());

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile));
    EXPECT_EQ(in.size(), 12u);
    EXPECT_TRUE(in.close());
}

UNIT_TEST(FileInputStream, PositionStartsAtZero) {
    const char *tmpfile = "tmp_pos_start_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write("hello"));
    EXPECT_TRUE(out.close());

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile));
    EXPECT_EQ(in.position(), 0u);
    EXPECT_TRUE(in.close());
}

UNIT_TEST(FileInputStream, PositionAdvancesOnRead) {
    const char *tmpfile = "tmp_pos_read_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write("hello world"));
    EXPECT_TRUE(out.close());

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile));
    char buf[5];
    EXPECT_TRUE(in.readOrFail(buf, 5));
    EXPECT_EQ(in.position(), 5u);
    EXPECT_TRUE(in.close());
}

UNIT_TEST(FileInputStream, PositionAdvancesOnSkip) {
    const char *tmpfile = "tmp_pos_skip_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    std::string data(2000, 'x');
    FileOutputStream out;
    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write(data));
    EXPECT_TRUE(out.close());

    // Use small buffer to test both buffered and seeked skips.
    FileInputStream in;
    ASSERT_TRUE(in.open(tmpfile, 128));

    (void) in.skip(50);
    EXPECT_EQ(in.position(), 50u);

    (void) in.skip(200); // Large skip via seek.
    EXPECT_EQ(in.position(), 250u);

    EXPECT_EQ(in.readAll().orThrow(), data.substr(250));
    EXPECT_EQ(in.position(), in.size());
    EXPECT_TRUE(in.close());
}

UNIT_TEST(FileInputStream, PositionAfterReadAll) {
    const char *tmpfile = "tmp_pos_readall_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write("hello"));
    EXPECT_TRUE(out.close());

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile));
    EXPECT_EQ(in.readAll().orThrow(), "hello");
    EXPECT_EQ(in.position(), 5u);
    EXPECT_EQ(in.position(), in.size());
    EXPECT_TRUE(in.close());
}

UNIT_TEST(FileInputStream, PositionResetsOnReopen) {
    const char *tmpfile1 = "tmp_pos_reopen1_test.txt";
    const char *tmpfile2 = "tmp_pos_reopen2_test.txt";
    ScopedTestFileSlot tmp1(tmpfile1);
    ScopedTestFileSlot tmp2(tmpfile2);

    FileOutputStream out1;

    ASSERT_TRUE(out1.open(tmpfile1));
    EXPECT_TRUE(out1.write("first"));
    EXPECT_TRUE(out1.close());

    FileOutputStream out2;

    ASSERT_TRUE(out2.open(tmpfile2));
    EXPECT_TRUE(out2.write("second!"));
    EXPECT_TRUE(out2.close());

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile1));
    (void) in.skip(3);
    EXPECT_EQ(in.position(), 3u);
    EXPECT_TRUE(in.close());

    EXPECT_TRUE(in.open(tmpfile2));
    EXPECT_EQ(in.position(), 0u);
    EXPECT_EQ(in.size(), 7u);
    EXPECT_TRUE(in.close());
}

