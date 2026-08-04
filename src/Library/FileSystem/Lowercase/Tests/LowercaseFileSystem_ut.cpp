#include <vector>
#include <memory>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Lowercase/LowercaseFileSystem.h"
#include "Library/FileSystem/Memory/MemoryFileSystem.h"
#include "Library/FileSystem/Directory/DirectoryFileSystem.h"

#include "Utility/ScopeGuard.h"

UNIT_TEST(LowercaseFileSystem, Empty) {
    MemoryFileSystem fs0("");
    LowercaseFileSystem fs(&fs0);

    EXPECT_TRUE(fs.ls("").orThrow().empty());
    EXPECT_TRUE(fs.exists("").orThrow());
    EXPECT_EQ(fs.stat("").orThrow(), FileStat(FILE_DIRECTORY, 0));
}

UNIT_TEST(LowercaseFileSystem, ExistsStatUppercase) {
    MemoryFileSystem fs0("");
    fs0.write("A.bin", Blob()).orThrow();

    LowercaseFileSystem fs(&fs0);
    EXPECT_FALSE(fs.exists("A.bin").orThrow());
    EXPECT_EQ(fs.stat("A.bin").orThrow(), FileStat());
}

UNIT_TEST(LowercaseFileSystem, KeepEmptyFolders) {
    MM_AT_SCOPE_EXIT(std::filesystem::remove_all("tmp_dir"));

    DirectoryFileSystem fs0("tmp_dir");
    fs0.write("a/b/c.bin", Blob()).orThrow();
    fs0.write("a/c/b.bin", Blob()).orThrow();

    // Check that LowercaseFileSystem keeps empty folders in this case.
    LowercaseFileSystem fs(&fs0);
    EXPECT_TRUE(fs.remove("a/b/c.bin").orThrow());
    EXPECT_TRUE(fs.remove("a/c/b.bin").orThrow());
    EXPECT_TRUE(fs0.exists("a/b").orThrow());
    EXPECT_TRUE(fs0.exists("a/c").orThrow());
    EXPECT_TRUE(fs.exists("a/b").orThrow());
    EXPECT_TRUE(fs.exists("a/c").orThrow());

    // Same check, just for a new LowercaseFileSystem for which we didn't call remove.
    LowercaseFileSystem fss(&fs0);
    EXPECT_TRUE(fss.exists("a/b").orThrow());
    EXPECT_TRUE(fss.exists("a/c").orThrow());
}

UNIT_TEST(LowercaseFileSystem, DropEmptyFolders) {
    MemoryFileSystem fs0("");
    fs0.write("a/b/c.bin", Blob()).orThrow();
    fs0.write("a/c/b.bin", Blob()).orThrow();

    // Check that LowercaseFileSystem drops empty folders in this case.
    LowercaseFileSystem fs(&fs0);
    EXPECT_TRUE(fs.remove("a/b/c.bin").orThrow());
    EXPECT_TRUE(fs.remove("a/c/b.bin").orThrow());
    EXPECT_FALSE(fs0.exists("a/b").orThrow());
    EXPECT_FALSE(fs0.exists("a/c").orThrow());
    EXPECT_FALSE(fs0.exists("a").orThrow());
    EXPECT_FALSE(fs.exists("a/b").orThrow());
    EXPECT_FALSE(fs.exists("a/c").orThrow());
    EXPECT_FALSE(fs.exists("a").orThrow());
}

UNIT_TEST(LowercaseFileSystem, Conflict) {
    MemoryFileSystem fs0("");
    fs0.write("A.bin", Blob()).orThrow();
    fs0.write("a.bin", Blob()).orThrow();

    LowercaseFileSystem fs(&fs0);
    EXPECT_TRUE(fs.exists("a.bin").orThrow());
    EXPECT_EQ(fs.stat("a.bin").orThrow(), FileStat(FILE_REGULAR, 0));
    EXPECT_EQ(fs.ls("").orThrow(), std::vector<DirectoryEntry>({{"a.bin", FILE_REGULAR}}));

    EXPECT_FALSE(fs.read("a.bin").ok());
    EXPECT_FALSE(fs.write("a.bin", Blob()).ok());
    EXPECT_FALSE(fs.openForReading("a.bin").ok());
    EXPECT_FALSE(fs.openForWriting("a.bin").ok());
    EXPECT_FALSE(fs.remove("a.bin").ok());
    EXPECT_FALSE(fs.rename("a.bin", "b.bin").ok());

    EXPECT_TRUE(fs0.exists("A.bin").orThrow());
    EXPECT_TRUE(fs0.exists("a.bin").orThrow());
}

UNIT_TEST(LowercaseFileSystem, ConflictFolders) {
    MemoryFileSystem fs0("");
    fs0.write("a/1", Blob()).orThrow();
    fs0.write("A/1", Blob()).orThrow();

    LowercaseFileSystem fs(&fs0);
    EXPECT_TRUE(fs.exists("a").orThrow());
    EXPECT_EQ(fs.stat("a").orThrow(), FileStat(FILE_REGULAR, 0));
    EXPECT_FALSE(fs.exists("a/1").orThrow());
    EXPECT_EQ(fs.stat("a/1").orThrow(), FileStat());
    EXPECT_EQ(fs.ls("").orThrow(), std::vector<DirectoryEntry>({{"a", FILE_REGULAR}}));
}

UNIT_TEST(LowercaseFileSystem, Lowercase) {
    MemoryFileSystem fs0("");
    fs0.write("A.bin", Blob::fromString("123")).orThrow();
    fs0.write("a/B/C/1.bin", Blob::fromString("321")).orThrow();
    fs0.write("a/C/C/1.bin", Blob::fromString("111")).orThrow();

    LowercaseFileSystem fs(&fs0);
    EXPECT_EQ(fs.read("a.bin").orThrow().str(), "123");
    EXPECT_EQ(fs.read("a/b/c/1.bin").orThrow().str(), "321");
    EXPECT_EQ(fs.read("a/c/c/1.bin").orThrow().str(), "111");
}

UNIT_TEST(LowercaseFileSystem, Shenanigans) {
    MemoryFileSystem fs0("");
    fs0.write("A/A/A.bin", Blob::fromString("123")).orThrow();

    LowercaseFileSystem fs(&fs0);
    EXPECT_TRUE(fs.exists("a/a/a.bin").orThrow());

    fs0.clear();
    EXPECT_EQ(fs.stat("a/a/a.bin").orThrow(), FileStat()); // stat() should call base->stat().
    EXPECT_EQ(fs.ls("a/a").orThrow(), std::vector<DirectoryEntry>({{"a.bin", FILE_REGULAR}}));

    // Check that we don't blow up in spectacular ways. Throwing is OK.
    EXPECT_FALSE(fs.read("a/a/a.bin").ok());
    EXPECT_FALSE(fs.openForReading("a/a/a.bin").ok());
}

UNIT_TEST(LowercaseFileSystem, Write) {
    MemoryFileSystem fs0("");
    fs0.write("B/B.bin", Blob::fromString("B")).orThrow();

    LowercaseFileSystem fs(&fs0);
    fs.write("a/a.bin", Blob::fromString("a")).orThrow();
    fs.write("b/b.bin", Blob::fromString("bbb")).orThrow();

    EXPECT_TRUE(fs.exists("a/a.bin").orThrow());
    EXPECT_TRUE(fs.exists("b/b.bin").orThrow());
    EXPECT_EQ(fs.ls("a").orThrow(), std::vector<DirectoryEntry>({{"a.bin", FILE_REGULAR}}));
    EXPECT_EQ(fs.ls("b").orThrow(), std::vector<DirectoryEntry>({{"b.bin", FILE_REGULAR}}));

    EXPECT_EQ(fs0.read("a/a.bin").orThrow().str(), "a");
    EXPECT_EQ(fs0.read("B/B.bin").orThrow().str(), "bbb");
}

UNIT_TEST(LowercaseFileSystem, PruneRemove) {
    MemoryFileSystem fs0("");
    fs0.write("a/a", Blob()).orThrow();

    LowercaseFileSystem fs(&fs0);
    EXPECT_TRUE(fs.remove("a/a").orThrow());

    EXPECT_FALSE(fs.exists("a").orThrow());
    EXPECT_TRUE(fs.ls("").orThrow().empty());
    EXPECT_FALSE(fs0.exists("a").orThrow()); // Because memory fs doesn't support empty dirs either.
}

UNIT_TEST(LowercaseFileSystem, PruneRename) {
    MemoryFileSystem fs0("");
    fs0.write("A/A/A", Blob::fromString("123")).orThrow();

    LowercaseFileSystem fs(&fs0);
    fs.rename("a/a/a", "b/b/b").orThrow();

    EXPECT_FALSE(fs.exists("a").orThrow()); // Pruning happened.
    EXPECT_TRUE(fs.exists("b").orThrow());
    EXPECT_EQ(fs.read("b/b/b").orThrow().str(), "123");

    EXPECT_FALSE(fs0.exists("A").orThrow());
    EXPECT_TRUE(fs0.exists("b").orThrow());
    EXPECT_EQ(fs0.read("b/b/b").orThrow().str(), "123");
}

UNIT_TEST(LowercaseFileSystem, RenameReplace) {
    MemoryFileSystem fs0("");
    fs0.write("A/A/A", Blob::fromString("AAA")).orThrow();
    fs0.write("B/B/B", Blob::fromString("BBB")).orThrow();

    LowercaseFileSystem fs(&fs0);
    fs.rename("a/a/a", "b/b/b").orThrow();

    EXPECT_FALSE(fs.exists("a").orThrow());
    EXPECT_TRUE(fs.exists("b").orThrow());
    EXPECT_EQ(fs.read("b/b/b").orThrow().str(), "AAA");

    EXPECT_FALSE(fs0.exists("A").orThrow());
    EXPECT_TRUE(fs0.exists("B").orThrow());
    EXPECT_EQ(fs0.read("B/B/B").orThrow().str(), "AAA");
}

UNIT_TEST(LowercaseFileSystem, RenameFolder) {
    MemoryFileSystem fs0("");
    fs0.write("A/A/A/A", Blob::fromString("AAAA")).orThrow();
    fs0.write("B/tmp", Blob()).orThrow();

    LowercaseFileSystem fs(&fs0);
    fs.rename("a/a", "b/b").orThrow();

    EXPECT_FALSE(fs.exists("a").orThrow());
    EXPECT_TRUE(fs.exists("b/b/a/a").orThrow());
    EXPECT_TRUE(fs.exists("b/tmp").orThrow());

    EXPECT_FALSE(fs0.exists("A").orThrow());
    EXPECT_TRUE(fs0.exists("B/b/A/A").orThrow());
    EXPECT_EQ(fs0.read("B/b/A/A").orThrow().str(), "AAAA");
}

UNIT_TEST(LowercaseFileSystem, WriteUppercase) {
    MemoryFileSystem fs0("");
    LowercaseFileSystem fs(&fs0);

    EXPECT_FALSE(fs.write("A", Blob()).ok());
    EXPECT_FALSE(fs.openForWriting("A").ok());
}

UNIT_TEST(LowercaseFileSystem, RenameRepeatedly) {
    MemoryFileSystem fs0("");
    fs0.write("A", Blob::fromString("A")).orThrow();

    LowercaseFileSystem fs(&fs0);
    fs.rename("a", "b").orThrow();
    fs.rename("b", "c").orThrow();
    fs.rename("c", "d").orThrow();
    fs.rename("d", "e/f/g/h").orThrow();
    fs.rename("e/f/g/h", "a").orThrow();

    EXPECT_TRUE(fs.exists("a").orThrow());
    EXPECT_EQ(fs.read("a").orThrow().str(), "A");
    EXPECT_EQ(fs.ls("").orThrow(), std::vector<DirectoryEntry>({{"a", FILE_REGULAR}}));
    EXPECT_FALSE(fs0.exists("A").orThrow());
    EXPECT_EQ(fs0.read("a").orThrow().str(), "A");
    EXPECT_EQ(fs0.ls("").orThrow(), std::vector<DirectoryEntry>({{"a", FILE_REGULAR}}));
}

UNIT_TEST(LowercaseFileSystem, RenameUppercase) {
    MemoryFileSystem fs0("");
    fs0.write("A", Blob::fromString("A")).orThrow();

    LowercaseFileSystem fs(&fs0);
    EXPECT_FALSE(fs.rename("a", "B").ok());
    EXPECT_TRUE(fs.exists("a").orThrow());
    EXPECT_EQ(fs.read("a").orThrow().str(), "A");
}

UNIT_TEST(LowercaseFileSystem, RemoveRepeatedly) {
    MemoryFileSystem fs0("");
    fs0.write("A", Blob::fromString("A")).orThrow();

    LowercaseFileSystem fs(&fs0);
    EXPECT_TRUE(fs.remove("a").orThrow());
    EXPECT_FALSE(fs.remove("a").orThrow());
    EXPECT_FALSE(fs.remove("a").orThrow());

    EXPECT_FALSE(fs.exists("a").orThrow());
    EXPECT_TRUE(fs.ls("").orThrow().empty());
    EXPECT_FALSE(fs0.exists("A").orThrow());
    EXPECT_TRUE(fs0.ls("").orThrow().empty());
}

UNIT_TEST(LowercaseFileSystem, DisplayPath) {
    MemoryFileSystem fs0("ram");
    fs0.write("A/A", Blob::fromString("A")).orThrow();

    LowercaseFileSystem fs(&fs0);

    Blob blob = fs.read("a/a").orThrow();
    EXPECT_EQ(blob.displayPath(), "ram://A/A");
    blob = {};

    std::unique_ptr<InputStream> input = fs.openForReading("a/a").orThrow();
    EXPECT_EQ(input->displayPath(), "ram://A/A");
    EXPECT_TRUE(input->close());

    std::unique_ptr<OutputStream> output = fs.openForWriting("a/a").orThrow();
    EXPECT_EQ(output->displayPath(), "ram://A/A");
    EXPECT_TRUE(output->close());

    EXPECT_EQ(fs.displayPath("a/b/c"), "ram://A/b/c");
    EXPECT_EQ(fs.displayPath(""), "ram://");
}

UNIT_TEST(LowercaseFileSystem, RemoveDeep) {
    MemoryFileSystem fs0("ram");
    fs0.write("A/B/0", Blob::fromString("0")).orThrow();
    fs0.write("A/B/1", Blob::fromString("1")).orThrow();

    LowercaseFileSystem fs(&fs0);

    EXPECT_TRUE(fs.remove("a/b/0").orThrow());
    EXPECT_FALSE(fs.exists("a/b/0").orThrow());
    EXPECT_FALSE(fs0.exists("A/B/0").orThrow());
    EXPECT_TRUE(fs.exists("a/b/1").orThrow());
    EXPECT_TRUE(fs0.exists("A/B/1").orThrow());
    EXPECT_EQ(fs.ls("a/b").orThrow(), std::vector<DirectoryEntry>({{"1", FILE_REGULAR}}));
    EXPECT_EQ(fs0.ls("A/B").orThrow(), std::vector<DirectoryEntry>({{"1", FILE_REGULAR}}));
}

UNIT_TEST(LowercaseFileSystem, RenameOverConflict) {
    MemoryFileSystem fs0("ram");
    fs0.write("A", Blob::fromString("")).orThrow();
    fs0.write("AAA", Blob::fromString("")).orThrow();
    fs0.write("AAa", Blob::fromString("")).orThrow();

    LowercaseFileSystem fs(&fs0);
    EXPECT_FALSE(fs.rename("a", "aaa/b").ok());
}

UNIT_TEST(LowercaseFileSystem, SelfRename) {
    MemoryFileSystem fs0("ram");
    fs0.write("ABC", Blob::fromString("abc")).orThrow();

    LowercaseFileSystem fs(&fs0);

    fs.rename("abc", "abc").orThrow();

    EXPECT_TRUE(fs.exists("abc").orThrow());
    EXPECT_EQ(fs.read("abc").orThrow().str(), "abc");
    EXPECT_EQ(fs.ls("").orThrow(), std::vector<DirectoryEntry>({{"abc", FILE_REGULAR}}));
}
