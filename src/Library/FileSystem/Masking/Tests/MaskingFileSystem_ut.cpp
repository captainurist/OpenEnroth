#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Masking/MaskingFileSystem.h"
#include "Library/FileSystem/Memory/MemoryFileSystem.h"

UNIT_TEST(MaskingFileSystem, SimpleMasks) {
    MemoryFileSystem fs0("");
    fs0.write("a/b/c/d", Blob()).orThrow();
    fs0.write("a/b/c/e", Blob()).orThrow();
    fs0.write("a/b/1/d", Blob()).orThrow();
    fs0.write("a/b/1/e", Blob()).orThrow();

    MaskingFileSystem fs1(&fs0);
    EXPECT_TRUE(fs1.exists("a").orThrow());
    EXPECT_TRUE(fs1.exists("a/b/c/d").orThrow());

    fs1.mask("a/b/c/d");
    EXPECT_FALSE(fs1.exists("a/b/c/d").orThrow());
    EXPECT_TRUE(fs0.exists("a/b/c/d").orThrow());
    EXPECT_EQ(fs1.ls("a/b/c").orThrow(), std::vector<DirectoryEntry>({{"e", FILE_REGULAR}}));

    fs1.mask("a/b/c/e");
    EXPECT_FALSE(fs1.exists("a/b/c/e").orThrow());
    EXPECT_TRUE(fs0.exists("a/b/c/e").orThrow());
    EXPECT_EQ(fs1.ls("a/b/c").orThrow(), std::vector<DirectoryEntry>()); // Masking might result in observable empty dirs.

    fs1.mask("");
    EXPECT_EQ(fs1.ls("").orThrow(), std::vector<DirectoryEntry>());

    EXPECT_TRUE(fs1.unmask("a/b/c/e"));
    EXPECT_FALSE(fs1.exists("a/b/c/e").orThrow()); // Still masked.
    EXPECT_TRUE(fs1.unmask(""));
    EXPECT_TRUE(fs1.exists("a/b/c/e").orThrow());
    EXPECT_FALSE(fs1.exists("a/b/c/d").orThrow());

    fs1.clearMasks();
    EXPECT_TRUE(fs1.exists("a/b/c/d").orThrow());
}

UNIT_TEST(MaskingFileSystem, PersistentMasking) {
    MemoryFileSystem fs0("");
    MaskingFileSystem fs1(&fs0);

    fs1.mask("a");
    fs0.write("a", Blob()).orThrow();
    EXPECT_FALSE(fs1.exists("a").orThrow());
    EXPECT_TRUE(fs0.exists("a").orThrow());
}
