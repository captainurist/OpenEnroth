#include <cstdlib>
#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/Streams/FileInputStream.h"

UNIT_TEST(FileOutputStream, Write) {
    const char *tmpfile = "tmp_test.txt";
    const char *tmpfilecontent = "1234\n";
    size_t tmpfilesize = strlen(tmpfilecontent);

    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write(tmpfilecontent, tmpfilesize));
    EXPECT_TRUE(out.close());

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile));

    char buf[1024] = {};
    size_t bytes = in.read(buf, 1024).orThrow();;
    EXPECT_EQ(bytes, 5);
    EXPECT_EQ(strcmp(buf, tmpfilecontent), 0);

    bytes = in.read(buf, 1024).orThrow();
    EXPECT_EQ(bytes, 0);
    EXPECT_TRUE(in.close());
}

UNIT_TEST(FileOutputStream, FlushMidStream) {
    const char *tmpfile = "tmp_flush_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write("hello"));
    EXPECT_TRUE(out.flush());

    {
        FileInputStream in;
        ASSERT_TRUE(in.open(tmpfile));
        EXPECT_EQ(in.readAll().orThrow(), "hello");
    }

    EXPECT_TRUE(out.write(" world"));
    EXPECT_TRUE(out.close());

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile));
    EXPECT_EQ(in.readAll().orThrow(), "hello world");
}

UNIT_TEST(FileOutputStream, LargeWriteBypassesBuffer) {
    // Use a small buffer so that a large write goes through the direct-write path in _overflow.
    const char *tmpfile = "tmp_largewrite_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile, 64));
    std::string large(1024, 'x');
    EXPECT_TRUE(out.write(large.data(), large.size()));
    EXPECT_TRUE(out.close());

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile));
    EXPECT_EQ(in.readAll().orThrow(), large);
}

UNIT_TEST(FileOutputStream, MixedSmallAndLargeWrites) {
    const char *tmpfile = "tmp_mixed_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile, 64));

    std::string expected;

    EXPECT_TRUE(out.write("hello"));
    expected += "hello";

    std::string large(256, 'y');
    EXPECT_TRUE(out.write(large.data(), large.size()));
    expected += large;

    EXPECT_TRUE(out.write(" end"));
    expected += " end";

    EXPECT_TRUE(out.close());

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile));
    EXPECT_EQ(in.readAll().orThrow(), expected);
}

UNIT_TEST(FileOutputStream, CloseIdempotent) {
    const char *tmpfile = "tmp_closeidem_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write("hello"));
    EXPECT_TRUE(out.close());
    EXPECT_FALSE(out.isOpen());
    EXPECT_TRUE(out.close()); // Double close is fine.
    EXPECT_FALSE(out.isOpen());
}

UNIT_TEST(FileOutputStream, ReopenAfterClose) {
    const char *tmpfile = "tmp_reopen_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write("first"));
    EXPECT_TRUE(out.close());

    EXPECT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write("second"));
    EXPECT_TRUE(out.close());

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile));
    EXPECT_EQ(in.readAll().orThrow(), "second"); // File is overwritten, not appended.
}

UNIT_TEST(FileOutputStream, PositionStartsAtZero) {
    const char *tmpfile = "tmp_pos_start_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_EQ(out.position(), 0u);
    EXPECT_TRUE(out.close());
}

UNIT_TEST(FileOutputStream, PositionAdvancesOnWrite) {
    const char *tmpfile = "tmp_pos_write_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write("hello"));
    EXPECT_EQ(out.position(), 5u);
    EXPECT_TRUE(out.write(" world"));
    EXPECT_EQ(out.position(), 11u);
    EXPECT_TRUE(out.close());
}

UNIT_TEST(FileOutputStream, PositionAfterFlush) {
    const char *tmpfile = "tmp_pos_flush_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write("hello"));
    EXPECT_TRUE(out.flush());
    EXPECT_EQ(out.position(), 5u);
    EXPECT_TRUE(out.write(" world"));
    EXPECT_EQ(out.position(), 11u);
    EXPECT_TRUE(out.close());
}

UNIT_TEST(FileOutputStream, PositionAfterLargeWrite) {
    const char *tmpfile = "tmp_pos_large_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile, 64));
    std::string large(1024, 'x');
    EXPECT_TRUE(out.write(large));
    EXPECT_EQ(out.position(), 1024u);
    EXPECT_TRUE(out.close());
}

UNIT_TEST(FileOutputStream, DestructorFlushesBuffer) {
    const char *tmpfile = "tmp_dtor_flush_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    {
        FileOutputStream out;
        ASSERT_TRUE(out.open(tmpfile));
        EXPECT_TRUE(out.write("hello"));
        // No explicit close() - destructor should flush.
    }

    FileInputStream in;

    ASSERT_TRUE(in.open(tmpfile));
    EXPECT_EQ(in.readAll().orThrow(), "hello");
}

UNIT_TEST(FileOutputStream, PositionResetsOnReopen) {
    const char *tmpfile = "tmp_pos_reopen_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out;

    ASSERT_TRUE(out.open(tmpfile));
    EXPECT_TRUE(out.write("hello"));
    EXPECT_EQ(out.position(), 5u);
    EXPECT_TRUE(out.close());

    EXPECT_TRUE(out.open(tmpfile));
    EXPECT_EQ(out.position(), 0u);
    EXPECT_TRUE(out.close());
}
