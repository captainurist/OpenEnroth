#include <ranges>
#include <utility>
#include <memory>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Memory/MemoryFileSystem.h"
#include "Library/FileSystem/Dump/FileSystemDump.h"

UNIT_TEST(MemoryFileSystem, EmptyRoot) {
    // Make sure accessing root works as expected.
    MemoryFileSystem fs("");
    EXPECT_TRUE(fs.ls("").orThrow().empty());
    EXPECT_TRUE(fs.exists("").orThrow());
    EXPECT_EQ(fs.stat("").orThrow(), FileStat(FILE_DIRECTORY, 0));

    EXPECT_FALSE(fs.read("").ok());
    EXPECT_FALSE(fs.write("", Blob()).ok());
    EXPECT_FALSE(fs.openForReading("").ok());
    EXPECT_FALSE(fs.openForWriting("").ok());
    EXPECT_FALSE(fs.remove("").ok());
    EXPECT_FALSE(fs.rename("", "").ok());
    EXPECT_FALSE(fs.rename("", "new").ok());

    fs.write("a/b.bin", Blob()).orThrow();
    EXPECT_FALSE(fs.rename("a", "").ok());
}

UNIT_TEST(MemoryFileSystem, Ls) {
    MemoryFileSystem fs("");

    fs.write("a/b", Blob()).orThrow();
    fs.write("a/c/d", Blob()).orThrow();

    EXPECT_EQ(dumpFileSystem(&fs), std::vector<FileSystemDumpEntry>({
        {"", FILE_DIRECTORY},
        {"a", FILE_DIRECTORY},
        {"a/b", FILE_REGULAR},
        {"a/c", FILE_DIRECTORY},
        {"a/c/d", FILE_REGULAR}
    }));

    EXPECT_FALSE(fs.ls("a/b").ok());
    EXPECT_FALSE(fs.ls("a/c/d").ok());
}

UNIT_TEST(MemoryFileSystem, ReadWrite) {
    MemoryFileSystem fs("");

    fs.write("a", Blob()).orThrow();
    EXPECT_EQ(fs.read("a").orThrow().size(), 0);

    EXPECT_FALSE(fs.write("a/b", Blob()).ok());

    fs.write("b", Blob::fromString("123")).orThrow();
    EXPECT_EQ(fs.read("b").orThrow().str(), "123");
}

UNIT_TEST(MemoryFileSystem, ReadDir) {
    MemoryFileSystem fs("");

    fs.write("a/b/c", Blob()).orThrow();
    EXPECT_FALSE(fs.read("a").ok());
    EXPECT_FALSE(fs.read("a/b").ok());
}

UNIT_TEST(MemoryFileSystem, ExistsStat) {
    MemoryFileSystem fs("");

    fs.write("a/b/c", Blob::fromString("123")).orThrow();
    EXPECT_TRUE(fs.exists("a").orThrow());
    EXPECT_TRUE(fs.exists("a/b").orThrow());
    EXPECT_TRUE(fs.exists("a/b/c").orThrow());
    EXPECT_EQ(fs.stat("a").orThrow(), FileStat(FILE_DIRECTORY, 0));
    EXPECT_EQ(fs.stat("a/b").orThrow(), FileStat(FILE_DIRECTORY, 0));
    EXPECT_EQ(fs.stat("a/b/c").orThrow(), FileStat(FILE_REGULAR, 3));
}

UNIT_TEST(MemoryFileSystem, Streaming) {
    MemoryFileSystem fs("");

    EXPECT_FALSE(fs.openForReading("a").ok());

    std::unique_ptr<OutputStream> output0 = fs.openForWriting("a").orThrow();
    EXPECT_TRUE(fs.exists("a").orThrow());
    EXPECT_EQ(fs.stat("a").orThrow(), FileStat(FILE_REGULAR, 0));
    EXPECT_FALSE(fs.read("a").ok());
    EXPECT_FALSE(fs.write("a", Blob()).ok());
    EXPECT_FALSE(fs.openForReading("a").ok());
    EXPECT_FALSE(fs.openForWriting("a").ok());

    EXPECT_TRUE(output0->write("123"));
    EXPECT_TRUE(output0->close());
    EXPECT_TRUE(fs.exists("a").orThrow());
    EXPECT_EQ(fs.stat("a").orThrow(), FileStat(FILE_REGULAR, 3));
    EXPECT_EQ(fs.read("a").orThrow().str(), "123");

    std::unique_ptr<InputStream> input0 = fs.openForReading("a").orThrow();
    std::unique_ptr<InputStream> input1 = fs.openForReading("a").orThrow();
    EXPECT_FALSE(fs.openForWriting("a").ok());
    EXPECT_FALSE(fs.write("a", Blob()).ok());
    EXPECT_EQ(fs.read("a").orThrow().str(), "123"); // read() works even when readers are active.

    EXPECT_EQ(input0->readAll().orThrow(), "123");
    EXPECT_EQ(input1->readAll().orThrow(), "123");
    EXPECT_FALSE(fs.openForWriting("a").ok()); // Still can't open for writing even when all read streams are at end.
    EXPECT_FALSE(fs.write("a", Blob()).ok());

    EXPECT_TRUE(input0->close());
    EXPECT_FALSE(fs.openForWriting("a").ok()); // One reader still active, can't write.
    EXPECT_FALSE(fs.write("a", Blob()).ok());

    EXPECT_TRUE(input1->close());
    fs.write("a", Blob()).orThrow();
    EXPECT_EQ(fs.stat("a").orThrow(), FileStat(FILE_REGULAR, 0));
}

UNIT_TEST(MemoryFileSystem, ReadBlobBlocksWriting) {
    // A Blob returned by read() shares the underlying memory with the file, so we treat it like an active reader -
    // any attempt to write to the same path must fail until the Blob is destroyed.
    MemoryFileSystem fs("");
    fs.write("a", Blob::fromString("123")).orThrow();

    Blob blob0 = fs.read("a").orThrow();
    EXPECT_EQ(blob0.str(), "123");
    EXPECT_FALSE(fs.write("a", Blob::fromString("456")).ok());
    EXPECT_FALSE(fs.openForWriting("a").ok());

    // Sharing / sub-blobbing extends the lifetime, so writes must still fail.
    Blob blob1 = Blob::share(blob0);
    Blob blob2 = blob0.subBlob(0, 1);
    blob0 = {};
    EXPECT_FALSE(fs.write("a", Blob::fromString("456")).ok());

    blob1 = {};
    EXPECT_FALSE(fs.write("a", Blob::fromString("456")).ok());

    // Once the last shared copy is gone, writing works again.
    blob2 = {};
    fs.write("a", Blob::fromString("456")).orThrow();
    EXPECT_EQ(fs.read("a").orThrow().str(), "456");
}

UNIT_TEST(MemoryFileSystem, Remove) {
    MemoryFileSystem fs("");

    fs.write("a", Blob::fromString("123")).orThrow();
    fs.write("b", Blob::fromString("456")).orThrow();
    std::unique_ptr<InputStream> input = fs.openForReading("a").orThrow();
    Blob blob = fs.read("b").orThrow();

    EXPECT_TRUE(fs.remove("a").orThrow());
    EXPECT_TRUE(fs.remove("b").orThrow());
    EXPECT_FALSE(fs.exists("a").orThrow());
    EXPECT_FALSE(fs.exists("b").orThrow());
    EXPECT_EQ(fs.ls("").orThrow().size(), 0);

    EXPECT_EQ(input->readAll().orThrow(), "123"); // Input stream still readable, even though the file was removed.
    EXPECT_TRUE(input->close());
    EXPECT_EQ(blob.str(), "456"); // Blob from read() still readable, even though the file was removed.
}

UNIT_TEST(MemoryFileSystem, Lifetime) {
    std::unique_ptr<MemoryFileSystem> fs = std::make_unique<MemoryFileSystem>("");

    fs->write("a", Blob::fromString("123")).orThrow();
    std::unique_ptr<InputStream> input = fs->openForReading("a").orThrow();
    std::unique_ptr<OutputStream> output = fs->openForWriting("b").orThrow();
    std::unique_ptr<OutputStream> output2 = fs->openForWriting("c").orThrow();

    fs.reset();
    EXPECT_EQ(input->readAll().orThrow(), "123"); // Input stream still readable, even though the FS was destroyed.
    EXPECT_TRUE(input->close());

    // Output stream still writeable & closeable.
    EXPECT_TRUE(output->write("123"));
    EXPECT_TRUE(output->close());

    // Closing in destructor also works.
    EXPECT_TRUE(output2->write("456"));
}

UNIT_TEST(MemoryFileSystem, DestructorFlushesData) {
    MemoryFileSystem fs("");

    {
        std::unique_ptr<OutputStream> output = fs.openForWriting("a").orThrow();
        EXPECT_TRUE(output->write("123"));
        // No explicit close() — destructor should flush the data.
    }

    EXPECT_EQ(fs.read("a").orThrow().str(), "123");
}

UNIT_TEST(MemoryFileSystem, Rename) {
    MemoryFileSystem fs("");

    fs.write("a/b/c", Blob::fromString("123")).orThrow();

    std::unique_ptr<InputStream> input = fs.openForReading("a/b/c").orThrow();
    std::unique_ptr<OutputStream> output = fs.openForWriting("a/b/d").orThrow();

    EXPECT_FALSE(fs.rename("a/b", "a/b/c").ok());
    EXPECT_FALSE(fs.rename("a/b", "a/b/d").ok());
    EXPECT_FALSE(fs.rename("a/b", "a/b/1").ok());
    EXPECT_FALSE(fs.rename("a/b", "a").ok());

    fs.rename("a/b", "x/y").orThrow();
    EXPECT_FALSE(fs.exists("a").orThrow()); // "a" is now empty, so was trimmed.
    EXPECT_FALSE(fs.ls("a").ok());

    EXPECT_EQ(input->readAll().orThrow(), "123"); // Moving files around keeps the streams valid.
    EXPECT_TRUE(output->write("1234"));
    EXPECT_TRUE(output->close());

    EXPECT_EQ(dumpFileSystem(&fs, FILE_SYSTEM_DUMP_WITH_CONTENTS), std::vector<FileSystemDumpEntry>({
        {"", FILE_DIRECTORY},
        {"x", FILE_DIRECTORY},
        {"x/y", FILE_DIRECTORY},
        {"x/y/c", FILE_REGULAR, "123"},
        {"x/y/d", FILE_REGULAR, "1234"}
    }));
}

UNIT_TEST(MemoryFileSystem, Overwrite) {
    MemoryFileSystem fs("");
    fs.write("a", Blob::fromString("a")).orThrow();

    std::unique_ptr<OutputStream> output = fs.openForWriting("a").orThrow();
    EXPECT_TRUE(output->write("A"));
    EXPECT_TRUE(output->close());

    EXPECT_EQ(fs.read("a").orThrow().str(), "A");
}

UNIT_TEST(MemoryFileSystem, DisplayPath) {
    MemoryFileSystem fs("mem");
    fs.write("a", Blob::fromString("a")).orThrow();

    EXPECT_EQ(fs.read("a").orThrow().displayPath(), "mem://a");

    std::unique_ptr<InputStream> input = fs.openForReading("a").orThrow();
    EXPECT_EQ(input->displayPath(), "mem://a");
    EXPECT_TRUE(input->close());

    std::unique_ptr<OutputStream> output = fs.openForWriting("b").orThrow();
    EXPECT_EQ(output->displayPath(), "mem://b");
    EXPECT_TRUE(output->close());

    // Also check that writing through a streaming interfaces preserves display path.
    EXPECT_EQ(fs.read("b").orThrow().displayPath(), "mem://b");
}

UNIT_TEST(MemoryFileSystem, ExceptionMessage) {
    MemoryFileSystem fs("mem");

    EXPECT_ERROR_MESSAGE(fs.read("a"), "mem://a");
    EXPECT_ERROR_MESSAGE(fs.openForReading("a"), "mem://a");
    EXPECT_ERROR_MESSAGE(fs.ls("a"), "mem://a");
}

UNIT_TEST(MemoryFileSystem, SelfRename) {
    MemoryFileSystem fs("");

    fs.write("a", Blob::fromString("123")).orThrow();
    fs.rename("a", "a").orThrow();

    EXPECT_TRUE(fs.exists("a").orThrow());
    EXPECT_EQ(fs.read("a").orThrow().str(), "123");

    fs.write("d/x", Blob::fromString("456")).orThrow();
    fs.rename("d", "d").orThrow();

    EXPECT_TRUE(fs.exists("d/x").orThrow());
    EXPECT_EQ(fs.read("d/x").orThrow().str(), "456");
}
