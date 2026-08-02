#include <cstdlib>
#include <string>
#include <filesystem>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/Streams/FileInputStream.h"
#include "Utility/Exception.h"

UNIT_TEST(FileOutputStream, Write) {
    const char *tmpfile = "tmp_test.txt";
    const char *tmpfilecontent = "1234\n";
    size_t tmpfilesize = strlen(tmpfilecontent);

    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write(tmpfilecontent, tmpfilesize);
    out.close();

    FileInputStream in(tmpfile);

    char buf[1024] = {};
    size_t bytes = in.read(buf, 1024);;
    EXPECT_EQ(bytes, 5);
    EXPECT_EQ(strcmp(buf, tmpfilecontent), 0);

    bytes = in.read(buf, 1024);
    EXPECT_EQ(bytes, 0);
    in.close();
}

UNIT_TEST(FileOutputStream, FlushMidStream) {
    const char *tmpfile = "tmp_flush_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello");
    out.flush();

    {
        FileInputStream in(tmpfile);
        EXPECT_EQ(in.readAll(), "hello");
    }

    out.write(" world");
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), "hello world");
}

UNIT_TEST(FileOutputStream, LargeWriteBypassesBuffer) {
    // Use a small buffer so that a large write goes through the direct-write path in _overflow.
    const char *tmpfile = "tmp_largewrite_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile, 64);
    std::string large(1024, 'x');
    out.write(large.data(), large.size());
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), large);
}

UNIT_TEST(FileOutputStream, MixedSmallAndLargeWrites) {
    const char *tmpfile = "tmp_mixed_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile, 64);

    std::string expected;

    out.write("hello");
    expected += "hello";

    std::string large(256, 'y');
    out.write(large.data(), large.size());
    expected += large;

    out.write(" end");
    expected += " end";

    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), expected);
}

UNIT_TEST(FileOutputStream, CloseIdempotent) {
    const char *tmpfile = "tmp_closeidem_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello");
    out.close();
    EXPECT_FALSE(out.isOpen());
    EXPECT_NO_THROW(out.close()); // Double close is fine.
    EXPECT_FALSE(out.isOpen());
}

UNIT_TEST(FileOutputStream, ReopenAfterClose) {
    const char *tmpfile = "tmp_reopen_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("first");
    out.close();

    out.open(tmpfile);
    out.write("second");
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), "second"); // File is overwritten, not appended.
}

UNIT_TEST(FileOutputStream, PositionStartsAtZero) {
    const char *tmpfile = "tmp_pos_start_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    EXPECT_EQ(out.position(), 0u);
    out.close();
}

UNIT_TEST(FileOutputStream, PositionAdvancesOnWrite) {
    const char *tmpfile = "tmp_pos_write_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello");
    EXPECT_EQ(out.position(), 5u);
    out.write(" world");
    EXPECT_EQ(out.position(), 11u);
    out.close();
}

UNIT_TEST(FileOutputStream, PositionAfterFlush) {
    const char *tmpfile = "tmp_pos_flush_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello");
    out.flush();
    EXPECT_EQ(out.position(), 5u);
    out.write(" world");
    EXPECT_EQ(out.position(), 11u);
    out.close();
}

UNIT_TEST(FileOutputStream, PositionAfterLargeWrite) {
    const char *tmpfile = "tmp_pos_large_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile, 64);
    std::string large(1024, 'x');
    out.write(large);
    EXPECT_EQ(out.position(), 1024u);
    out.close();
}

UNIT_TEST(FileOutputStream, DestructorFlushesBuffer) {
    const char *tmpfile = "tmp_dtor_flush_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    {
        FileOutputStream out(tmpfile);
        out.write("hello");
        // No explicit close() - destructor should flush.
    }

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), "hello");
}

UNIT_TEST(FileOutputStream, WriteCrossingBufferBoundary) {
    const char *tmpfile = "tmp_crossboundary_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    // The second write doesn't fit into what's left of the buffer, but is still smaller than the buffer itself, so
    // it goes through the head/tail split in `_overflow`.
    std::string first(50, 'a');
    std::string second(30, 'b');

    FileOutputStream out(tmpfile, 64);
    out.write(first);
    out.write(second);
    EXPECT_EQ(out.position(), 80u);
    out.close();

    EXPECT_EQ(std::filesystem::file_size(tmpfile), 80u);

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), first + second);
}

UNIT_TEST(FileOutputStream, ManySmallWritesAcrossManyBuffers) {
    const char *tmpfile = "tmp_manysmall_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    std::string expected;

    FileOutputStream out(tmpfile, 64);
    for (int i = 0; i < 500; i++) {
        std::string chunk(7, static_cast<char>('a' + (i % 26)));
        out.write(chunk);
        expected += chunk;
    }
    EXPECT_EQ(out.position(), 3500u);
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), expected);
}

UNIT_TEST(FileOutputStream, WriteAtBufferSizeBoundary) {
    const char *tmpfile1 = "tmp_writeboundary1_test.txt";
    const char *tmpfile2 = "tmp_writeboundary2_test.txt";
    ScopedTestFileSlot tmp1(tmpfile1);
    ScopedTestFileSlot tmp2(tmpfile2);

    // `_overflow` switches between the buffered and the direct path at exactly `bufferSize`.
    std::string exact(64, 'x');

    FileOutputStream out1(tmpfile1, 64);
    out1.write(exact);
    out1.close();

    FileInputStream in1(tmpfile1);
    EXPECT_EQ(in1.readAll(), exact);

    std::string head(10, 'h');

    FileOutputStream out2(tmpfile2, 64);
    out2.write(head);
    out2.write(exact);
    EXPECT_EQ(out2.position(), 74u);
    out2.close();

    FileInputStream in2(tmpfile2);
    EXPECT_EQ(in2.readAll(), head + exact);
}

UNIT_TEST(FileOutputStream, TruncatesLongerExistingFile) {
    const char *tmpfile = "tmp_truncate_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out1(tmpfile);
    out1.write(std::string(100, 'x'));
    out1.close();
    EXPECT_EQ(std::filesystem::file_size(tmpfile), 100u);

    FileOutputStream out2(tmpfile);
    out2.write("ab");
    out2.close();
    EXPECT_EQ(std::filesystem::file_size(tmpfile), 2u);
}

UNIT_TEST(FileOutputStream, ExceptionMessages) {
    const char *path = "no_such_dir_qwezxc/tmp_test.txt";

    EXPECT_FALSE(std::filesystem::exists("no_such_dir_qwezxc"));
    EXPECT_THROW_MESSAGE(FileOutputStream out(path), "no_such_dir_qwezxc");
}

UNIT_TEST(FileOutputStream, DoubleFlushDoesNotDuplicate) {
    const char *tmpfile = "tmp_doubleflush_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello");
    out.flush();
    out.flush(); // Must not write "hello" out for the second time.
    out.write(" world");
    out.close();

    EXPECT_EQ(std::filesystem::file_size(tmpfile), 11u);

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), "hello world");
}

UNIT_TEST(FileOutputStream, FlushMakesFileSizeMatchPosition) {
    const char *tmpfile = "tmp_flushsize_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile, 64);
    out.write(std::string(200, 'a'));
    out.flush();
    EXPECT_EQ(std::filesystem::file_size(tmpfile), out.position());

    out.write(std::string(10, 'b'));
    out.flush();
    EXPECT_EQ(std::filesystem::file_size(tmpfile), out.position());
    out.close();
}

UNIT_TEST(FileOutputStream, BinaryModeNoCrlfExpansion) {
    const char *tmpfile = "tmp_binarymode_test.bin";
    ScopedTestFileSlot tmp(tmpfile);

    // On Windows a text-mode stream would expand '\n' into "\r\n" and treat 0x1A as end of file.
    std::string data = "a\r\nb\x1A" "c";
    EXPECT_EQ(data.size(), 6u);

    FileOutputStream out(tmpfile);
    out.write(data);
    out.close();

    EXPECT_EQ(std::filesystem::file_size(tmpfile), 6u);

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), data);
}

UNIT_TEST(FileOutputStream, EmptyFileCreatedOnOpen) {
    const char *tmpfile = "tmp_emptycreate_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    {
        FileOutputStream out(tmpfile);
        out.close();
    }

    EXPECT_TRUE(std::filesystem::exists(tmpfile));
    EXPECT_EQ(std::filesystem::file_size(tmpfile), 0u);
}

UNIT_TEST(FileOutputStream, DestructorFlushesAfterLargeWrite) {
    const char *tmpfile = "tmp_dtorlarge_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    std::string large(500, 'x');
    std::string tail(10, 'y');

    {
        FileOutputStream out(tmpfile, 64);
        out.write(large); // Goes through the direct-write path.
        out.write(tail);  // Stays in the buffer.
        // No explicit close() - destructor should flush the tail.
    }

    EXPECT_EQ(std::filesystem::file_size(tmpfile), 510u);

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), large + tail);
}

UNIT_TEST(FileOutputStream, ReopenWithSmallerBufferSize) {
    const char *tmpfile1 = "tmp_outreopenbuf1_test.txt";
    const char *tmpfile2 = "tmp_outreopenbuf2_test.txt";
    ScopedTestFileSlot tmp1(tmpfile1);
    ScopedTestFileSlot tmp2(tmpfile2);

    std::string data(2000, 'z');

    // The buffer allocated for the first stream must not be reused for the second one.
    FileOutputStream out(tmpfile1, 4096);
    out.write("first");
    out.close();

    out.open(tmpfile2, 64);
    out.write(data);
    out.close();

    FileInputStream in(tmpfile2);
    EXPECT_EQ(in.readAll(), data);
}

UNIT_TEST(FileOutputStream, DefaultConstructedThenOpen) {
    const char *tmpfile = "tmp_outdefaultctor_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    { FileOutputStream out; } // Destructing a never-opened stream is fine.

    FileOutputStream out;
    EXPECT_FALSE(out.isOpen());
    out.open(tmpfile);
    EXPECT_TRUE(out.isOpen());
    out.write("hello");
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), "hello");
}

UNIT_TEST(FileOutputStream, UnicodePath) {
    std::u8string u8path = u8"файл_output.txt"; // "File" in Russian.
    std::string path = reinterpret_cast<const char *>(u8path.c_str());
    ScopedTestFileSlot tmp(path);

    std::string data(300, 'w');

    FileOutputStream out(path);
    out.write(data);
    out.close();

    // Check with the UTF-8 api directly, without going through our own classes.
    EXPECT_TRUE(std::filesystem::exists(u8path));
    EXPECT_EQ(std::filesystem::file_size(u8path), data.size());
}

#ifdef __linux__
UNIT_TEST(FileOutputStream, PositionSurvivesFailedWrite) {
    // `/dev/full` can be opened for writing, but every write into it fails with `ENOSPC`. This is the only portable
    // enough way to exercise the failing-write paths without messing with process-wide state like `RLIMIT_FSIZE`.
    FileOutputStream out("/dev/full", 64);

    out.write(std::string(63, 'a')); // Buffered, no IO yet.
    EXPECT_EQ(out.position(), 63u);

    EXPECT_THROW(out.flush(), Exception);
    EXPECT_EQ(out.position(), 63u); // Must not jump backwards just because the flush failed.

    out.write("XYZ");
    EXPECT_EQ(out.position(), 66u);

    EXPECT_NO_THROW(out.close());
}

UNIT_TEST(FileOutputStream, FailedWriteIsNotRetried) {
    // A failing write must not be retried from `_close`, as that would duplicate whatever prefix did make it out.
    FileOutputStream out("/dev/full", 64);

    std::string large(200, 'a');
    EXPECT_THROW(out.write(large), Exception); // Goes through the direct-write path in `_overflow`.
    EXPECT_NO_THROW(out.close());
}

UNIT_TEST(FileOutputStream, FailedCloseStillClosesTheStream) {
    FileOutputStream out("/dev/full", 64);
    out.write("hello"); // Small enough to stay in the buffer, so it's `close()` that fails to write it out.

    EXPECT_THROW(out.close(), Exception);

    // Even though flushing has failed, the stream is closed and the handle is released - otherwise the destructor
    // would re-enter `_close` and retry the failed write into a live handle.
    EXPECT_FALSE(out.isOpen());
}

UNIT_TEST(FileOutputStream, FailedWriteInDestructorDoesNotThrow) {
    EXPECT_NO_THROW({
        FileOutputStream out("/dev/full", 64);
        out.write("hello"); // Stays in the buffer, so the destructor is the one that fails to write it out.
    });
}
#endif

UNIT_TEST(FileOutputStream, PositionResetsOnReopen) {
    const char *tmpfile = "tmp_pos_reopen_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello");
    EXPECT_EQ(out.position(), 5u);
    out.close();

    out.open(tmpfile);
    EXPECT_EQ(out.position(), 0u);
    out.close();
}
