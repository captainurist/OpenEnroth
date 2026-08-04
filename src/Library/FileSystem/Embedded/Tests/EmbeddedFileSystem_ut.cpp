#include <vector>
#include <memory>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Embedded/EmbeddedFileSystem.h"

CMRC_DECLARE(testrc);

UNIT_TEST(EmbeddedFileSystem, StatExists) {
    EmbeddedFileSystem fs(cmrc::testrc::get_filesystem(), "testrc");

    EXPECT_TRUE(fs.exists("Tests").orThrow());
    EXPECT_EQ(fs.stat("Tests").orThrow(), FileStat(FILE_DIRECTORY, 0));

    EXPECT_TRUE(fs.exists("Tests/EmbeddedFileSystem_ut.cpp").orThrow());
    EXPECT_EQ(fs.stat("Tests/EmbeddedFileSystem_ut.cpp").orThrow().type, FILE_REGULAR);

    EXPECT_FALSE(fs.exists("DoesntExist").orThrow());
    EXPECT_EQ(fs.stat("DoesntExist").orThrow(), FileStat(FILE_INVALID, 0));
}

UNIT_TEST(EmbeddedFileSystem, Ls) {
    EmbeddedFileSystem fs(cmrc::testrc::get_filesystem(), "testrc");

    EXPECT_EQ(fs.ls("").orThrow(), std::vector<DirectoryEntry>({{"Tests", FILE_DIRECTORY}}));
    EXPECT_EQ(fs.ls("Tests").orThrow(), std::vector<DirectoryEntry>({{"EmbeddedFileSystem_ut.cpp", FILE_REGULAR}}));

    EXPECT_FALSE(fs.ls("DoesntExist").ok());
}

UNIT_TEST(EmbeddedFileSystem, Read) {
    EmbeddedFileSystem fs(cmrc::testrc::get_filesystem(), "testrc");

    const char *needle = "123456789012345678901234567890";

    Blob data = fs.read("Tests/EmbeddedFileSystem_ut.cpp").orThrow();
    EXPECT_CONTAINS(data.str(), needle);

    std::unique_ptr<InputStream> input = fs.openForReading("Tests/EmbeddedFileSystem_ut.cpp").orThrow();
    EXPECT_CONTAINS(input->readAll().orThrow(), needle);

    EXPECT_FALSE(fs.read("DoesntExist").ok());
    EXPECT_FALSE(fs.openForReading("DoesntExist").ok());
}

UNIT_TEST(EmbeddedFileSystem, DisplayPath) {
    EmbeddedFileSystem fs(cmrc::testrc::get_filesystem(), "testrc");

    Blob data = fs.read("Tests/EmbeddedFileSystem_ut.cpp").orThrow();
    EXPECT_EQ(data.displayPath(), "testrc://Tests/EmbeddedFileSystem_ut.cpp");

    std::unique_ptr<InputStream> input = fs.openForReading("Tests/EmbeddedFileSystem_ut.cpp").orThrow();
    EXPECT_EQ(input->displayPath(), "testrc://Tests/EmbeddedFileSystem_ut.cpp");
}
