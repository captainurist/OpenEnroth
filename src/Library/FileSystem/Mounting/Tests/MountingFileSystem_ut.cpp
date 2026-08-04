#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Mounting/MountingFileSystem.h"
#include "Library/FileSystem/Memory/MemoryFileSystem.h"
#include "Library/FileSystem/Merging/MergingFileSystem.h"
#include "Library/FileSystem/Null/NullFileSystem.h"
#include "Library/FileSystem/Dump/FileSystemDump.h"

UNIT_TEST(MountingFileSystem, StatExists) {
    MemoryFileSystem mfs("");
    mfs.write("c/d", Blob()).orThrow();

    MountingFileSystem fs("");
    fs.mount("a/b", &mfs);

    EXPECT_TRUE(fs.exists("a").orThrow());
    EXPECT_TRUE(fs.exists("a/b").orThrow());
    EXPECT_TRUE(fs.exists("a/b/c").orThrow());
    EXPECT_TRUE(fs.exists("a/b/c/d").orThrow());

    EXPECT_EQ(fs.stat("a").orThrow(), FileStat(FILE_DIRECTORY, 0));
    EXPECT_EQ(fs.stat("a/b").orThrow(), FileStat(FILE_DIRECTORY, 0));
    EXPECT_EQ(fs.stat("a/b/c").orThrow(), FileStat(FILE_DIRECTORY, 0));
    EXPECT_EQ(fs.stat("a/b/c/d").orThrow(), FileStat(FILE_REGULAR, 0));
}

UNIT_TEST(MountingFileSystem, Override) {
    MemoryFileSystem mfs("");
    mfs.write("a", Blob()).orThrow();

    MountingFileSystem fs("");
    fs.mount("", &mfs);
    fs.mount("a", &mfs);

    EXPECT_TRUE(fs.exists("a").orThrow());
    EXPECT_TRUE(fs.exists("a/a").orThrow());
    EXPECT_EQ(fs.stat("a").orThrow(), FileStat(FILE_DIRECTORY, 0));
    EXPECT_EQ(fs.stat("a/a").orThrow(), FileStat(FILE_REGULAR, 0));

    EXPECT_EQ(fs.ls("").orThrow(), std::vector<DirectoryEntry>({{"a", FILE_DIRECTORY}}));
    EXPECT_EQ(fs.ls("a").orThrow(), std::vector<DirectoryEntry>({{"a", FILE_REGULAR}}));
}

UNIT_TEST(MountingFileSystem, SchrodingerOverride) {
    MemoryFileSystem mfs1("");
    MemoryFileSystem mfs2("");
    MergingFileSystem sfs({&mfs1, &mfs2});

    MountingFileSystem fs("");
    fs.mount("", &sfs);

    mfs1.write("a/a.bin", Blob()).orThrow();
    mfs2.write("a", Blob()).orThrow();
    EXPECT_EQ(dumpFileSystem(&fs), std::vector<FileSystemDumpEntry>({
        {"", FILE_DIRECTORY},
        {"a", FILE_REGULAR},
        {"a", FILE_DIRECTORY},
        {"a/a.bin", FILE_REGULAR}
    }));

    NullFileSystem nfs;
    fs.mount("a", &nfs);
    EXPECT_EQ(dumpFileSystem(&fs), std::vector<FileSystemDumpEntry>({
        {"", FILE_DIRECTORY},
        {"a", FILE_DIRECTORY}
    }));
}

UNIT_TEST(MountingFileSystem, SimpleMerge) {
    MemoryFileSystem mfs1("");
    MemoryFileSystem mfs2("");

    MountingFileSystem fs("");
    fs.mount("", &mfs1);
    fs.mount("a", &mfs2);

    mfs1.write("b", Blob::fromString("b")).orThrow();
    mfs2.write("a", Blob::fromString("a")).orThrow();

    EXPECT_EQ(dumpFileSystem(&fs, FILE_SYSTEM_DUMP_WITH_CONTENTS), std::vector<FileSystemDumpEntry>({
        {"", FILE_DIRECTORY},
        {"a", FILE_DIRECTORY},
        {"a/a", FILE_REGULAR, "a"},
        {"b", FILE_REGULAR, "b"}
    }));
}

UNIT_TEST(MountingFileSystem, WriteIntoVfs) {
    MemoryFileSystem mfs("");

    MountingFileSystem fs("");
    fs.mount("a/b/c/d", &mfs);
    fs.mount("a/b/g/e", &mfs);

    EXPECT_FALSE(fs.write("a/b/1.bin", Blob()).ok());
    EXPECT_FALSE(fs.openForWriting("a/b/g/1.bin").ok());
}

UNIT_TEST(MountingFileSystem, ReadWriteThrough) {
    MemoryFileSystem mfs("");

    MountingFileSystem fs("");
    fs.mount("a", &mfs);
    fs.mount("a/b", &mfs);
    fs.mount("x", &mfs);

    fs.write("a/b/b", Blob::fromString("123")).orThrow();

    EXPECT_EQ(mfs.stat("b").orThrow(), FileStat(FILE_REGULAR, 3));
    EXPECT_EQ(fs.stat("a/b").orThrow(), FileStat(FILE_DIRECTORY, 0));
    EXPECT_EQ(fs.read("x/b").orThrow().str(), "123");
    EXPECT_EQ(fs.read("a/b/b").orThrow().str(), "123");
}

UNIT_TEST(MountingFileSystem, Remove) {
    MemoryFileSystem mfs("");

    MountingFileSystem fs("");
    fs.mount("a", &mfs);

    mfs.write("a", Blob()).orThrow();

    EXPECT_FALSE(fs.remove("a").ok());
    EXPECT_TRUE(fs.remove("a/a").orThrow());
    EXPECT_FALSE(fs.remove("a/a").orThrow());
}

UNIT_TEST(MountingFileSystem, RenameSameFs) {
    MemoryFileSystem mfs("");

    MountingFileSystem fs("");
    fs.mount("a", &mfs);

    mfs.write("a", Blob::fromString("123")).orThrow();

    fs.rename("a/a", "a/b").orThrow();

    EXPECT_EQ(mfs.read("b").orThrow().str(), "123");
    EXPECT_EQ(fs.read("a/b").orThrow().str(), "123");
    EXPECT_EQ(fs.ls("a").orThrow(), std::vector<DirectoryEntry>({{"b", FILE_REGULAR}}));
}

UNIT_TEST(MountingFileSystem, RenameDifferentFs) {
    MemoryFileSystem mfs1("");
    MemoryFileSystem mfs2("");

    MountingFileSystem fs("");
    fs.mount("1", &mfs1);
    fs.mount("2", &mfs2);

    mfs1.write("a", Blob::fromString("123")).orThrow();

    fs.rename("1/a", "2/a").orThrow();

    EXPECT_FALSE(mfs1.exists("a").orThrow());
    EXPECT_TRUE(mfs2.exists("a").orThrow());
    EXPECT_EQ(mfs2.read("a").orThrow().str(), "123");
}

UNIT_TEST(MountingFileSystem, Binary) {
    MountingFileSystem fs("");
    fs.mount("0", &fs);
    fs.mount("1", &fs);

    EXPECT_TRUE(fs.exists("1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1").orThrow());
    EXPECT_TRUE(fs.exists("0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0").orThrow());
}

UNIT_TEST(MountingFileSystem, LsMergeWithManyVirtualDirs) {
    // MountingFileSystem::_ls used to capture a std::span from the entries vector, then push_back-ing new
    // entries for virtual directories could cause vector reallocation, making the span dangling.
    MemoryFileSystem base("");
    base.write("x", Blob()).orThrow();

    // Create many separate mount points so the trie has many children at root level.
    // These virtual dirs don't exist in the base FS, forcing push_back for each one.
    std::vector<std::unique_ptr<MemoryFileSystem>> mounts;
    for (size_t i = 0; i < 100; i++)
        mounts.push_back(std::make_unique<MemoryFileSystem>(""));

    MountingFileSystem fs("");
    fs.mount("", &base);
    for (size_t i = 0; i < mounts.size(); i++)
        fs.mount(std::to_string(i), mounts[i].get());

    std::vector<DirectoryEntry> entries = fs.ls("").orThrow();
    EXPECT_EQ(entries.size(), 101);

    for (size_t i = 0; i < mounts.size(); i++)
        EXPECT_TRUE(std::ranges::contains(entries, DirectoryEntry(std::to_string(i), FILE_DIRECTORY)));
    EXPECT_TRUE(std::ranges::contains(entries, DirectoryEntry("x", FILE_REGULAR)));
}

