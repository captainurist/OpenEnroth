#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Memory/MemoryFileSystem.h"
#include "Library/FileSystem/Sub/SubFileSystem.h"

UNIT_TEST(SubFileSystem, ReadFile) {
    MemoryFileSystem base("memfs");
    base.write("dir/file.txt", Blob::fromString("hello")).orThrow();

    SubFileSystem sub("dir", &base);

    EXPECT_TRUE(sub.exists("file.txt").orThrow());
    Blob content = sub.read("file.txt").orThrow();
    EXPECT_EQ(content.str(), "hello");
}

UNIT_TEST(SubFileSystem, FileNotFound) {
    MemoryFileSystem base("memfs");
    SubFileSystem sub("dir", &base);

    EXPECT_FALSE(sub.exists("file.txt").orThrow());
}

UNIT_TEST(SubFileSystem, ListDirectory) {
    MemoryFileSystem base("memfs");
    base.write("shaders/a.vert", Blob::fromString("a")).orThrow();
    base.write("shaders/b.frag", Blob::fromString("b")).orThrow();

    SubFileSystem sub("shaders", &base);

    auto entries = sub.ls("").orThrow();
    EXPECT_EQ(entries.size(), 2);
}

UNIT_TEST(SubFileSystem, NestedDirectory) {
    MemoryFileSystem base("memfs");
    base.write("shaders/include/common.vert", Blob::fromString("common")).orThrow();

    SubFileSystem sub("shaders", &base);

    EXPECT_TRUE(sub.exists("include/common.vert").orThrow());
    Blob content = sub.read("include/common.vert").orThrow();
    EXPECT_EQ(content.str(), "common");
}

UNIT_TEST(SubFileSystem, DisplayPath) {
    MemoryFileSystem base("memfs");
    base.write("shaders/test.vert", Blob::fromString("test")).orThrow();

    SubFileSystem sub("shaders", &base);

    std::string path = sub.displayPath("test.vert");
    EXPECT_CONTAINS(path, "shaders");
    EXPECT_CONTAINS(path, "test.vert");
}

UNIT_TEST(SubFileSystem, CannotEscapeWithDotDot) {
    MemoryFileSystem base("memfs");
    base.write("secret.txt", Blob::fromString("secret")).orThrow();
    base.write("shaders/test.vert", Blob::fromString("test")).orThrow();

    SubFileSystem sub("shaders", &base);

    // Should not be able to access files outside the sub directory.
    EXPECT_FALSE(sub.exists("../secret.txt").orThrow());
    EXPECT_FALSE(sub.read("../secret.txt").ok());
    EXPECT_FALSE(sub.openForReading("../secret.txt").ok());
}
