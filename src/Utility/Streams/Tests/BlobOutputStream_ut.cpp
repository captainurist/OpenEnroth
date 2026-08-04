#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Memory/Blob.h"
#include "Utility/Streams/BlobOutputStream.h"

UNIT_TEST(BlobOutputStream, DestructorFlushesData) {
    Blob blob;
    {
        BlobOutputStream output(&blob);
        EXPECT_TRUE(output.write("hello"));
        EXPECT_TRUE(output.write("world"));
    }
    EXPECT_EQ(blob.str(), "helloworld");
}

UNIT_TEST(BlobOutputStream, MultipleChunks) {
    Blob blob;
    BlobOutputStream output(&blob);

    std::string expected;
    for (int i = 0; i < 10000; i++) {
        std::string chunk = std::to_string(i) + " ";
        EXPECT_TRUE(output.write(chunk.data(), chunk.size()));
        expected += chunk;
    }

    EXPECT_TRUE(output.close());
    EXPECT_EQ(blob.str(), expected);
}

UNIT_TEST(BlobOutputStream, CloseWithoutWriting) {
    Blob blob = Blob::fromString("old");
    BlobOutputStream output(&blob);
    EXPECT_TRUE(output.close());
    EXPECT_EQ(blob.size(), 0u);
}

UNIT_TEST(BlobOutputStream, FlushMidStream) {
    Blob blob;
    BlobOutputStream output(&blob);
    EXPECT_TRUE(output.write("hello"));
    EXPECT_TRUE(output.flush());
    EXPECT_EQ(blob.str(), "hello");

    EXPECT_TRUE(output.write(" world"));
    EXPECT_TRUE(output.close());
    EXPECT_EQ(blob.str(), "hello world");
}

UNIT_TEST(BlobOutputStream, DisplayPath) {
    Blob blob;
    BlobOutputStream output(&blob, "test.bin");
    EXPECT_TRUE(output.write("data"));
    EXPECT_TRUE(output.close());
    EXPECT_EQ(blob.displayPath(), "test.bin");
}

UNIT_TEST(BlobOutputStream, DisplayPathEmptyStream) {
    Blob blob;
    BlobOutputStream output(&blob, "empty.bin");
    EXPECT_TRUE(output.close());
    EXPECT_EQ(blob.displayPath(), "empty.bin");
}

UNIT_TEST(BlobOutputStream, CloseIdempotent) {
    Blob blob;
    BlobOutputStream output(&blob);
    EXPECT_TRUE(output.write("hello"));
    EXPECT_TRUE(output.close());
    EXPECT_FALSE(output.isOpen());
    EXPECT_TRUE(output.close()); // Double close is fine.
    EXPECT_FALSE(output.isOpen());
    EXPECT_EQ(blob.str(), "hello");
}

UNIT_TEST(BlobOutputStream, ReopenAfterClose) {
    Blob blob;
    BlobOutputStream output(&blob);
    EXPECT_TRUE(output.write("first"));
    EXPECT_TRUE(output.close());
    EXPECT_EQ(blob.str(), "first");

    output.open(&blob);
    EXPECT_TRUE(output.write("second"));
    EXPECT_TRUE(output.close());
    EXPECT_EQ(blob.str(), "second");
}

UNIT_TEST(BlobOutputStream, LargeWrite) {
    Blob blob;
    BlobOutputStream output(&blob);

    std::string large(8192, 'x');
    EXPECT_TRUE(output.write(large.data(), large.size()));
    EXPECT_TRUE(output.close());
    EXPECT_EQ(blob.str(), large);
}

UNIT_TEST(BlobOutputStream, GrowthCap) {
    // Chunks grow geometrically (1KB, 2KB, ..., 1MB cap). Write ~3MB to hit the cap.
    Blob blob;
    BlobOutputStream output(&blob);

    std::string expected;
    std::string chunk(1024, 'a');
    for (int i = 0; i < 3072; i++) {
        chunk[0] = static_cast<char>('a' + (i % 26));
        EXPECT_TRUE(output.write(chunk.data(), chunk.size()));
        expected.append(chunk);
    }
    EXPECT_TRUE(output.close());
    EXPECT_EQ(blob.str(), expected);
}

UNIT_TEST(BlobOutputStream, WriteZero) {
    Blob blob;
    BlobOutputStream output(&blob);
    EXPECT_TRUE(output.write("hello"));
    EXPECT_TRUE(output.write(nullptr, 0));
    EXPECT_TRUE(output.write(" world"));
    EXPECT_TRUE(output.close());
    EXPECT_EQ(blob.str(), "hello world");
}

UNIT_TEST(BlobOutputStream, PositionStartsAtZero) {
    Blob blob;
    BlobOutputStream output(&blob);
    EXPECT_EQ(output.position(), 0u);
    EXPECT_TRUE(output.close());
}

UNIT_TEST(BlobOutputStream, PositionAdvancesOnWrite) {
    Blob blob;
    BlobOutputStream output(&blob);
    EXPECT_TRUE(output.write("hello"));
    EXPECT_EQ(output.position(), 5u);
    EXPECT_TRUE(output.write(" world"));
    EXPECT_EQ(output.position(), 11u);
    EXPECT_TRUE(output.close());
}

UNIT_TEST(BlobOutputStream, PositionAfterFlush) {
    Blob blob;
    BlobOutputStream output(&blob);
    EXPECT_TRUE(output.write("hello"));
    EXPECT_TRUE(output.flush());
    EXPECT_EQ(output.position(), 5u);
    EXPECT_TRUE(output.write(" world"));
    EXPECT_EQ(output.position(), 11u);
    EXPECT_TRUE(output.close());
}

UNIT_TEST(BlobOutputStream, PositionAfterLargeWrite) {
    Blob blob;
    BlobOutputStream output(&blob);
    std::string large(8192, 'x');
    EXPECT_TRUE(output.write(large));
    EXPECT_EQ(output.position(), 8192u);
    EXPECT_TRUE(output.close());
}

UNIT_TEST(BlobOutputStream, PositionResetsOnReopen) {
    Blob blob;
    BlobOutputStream output(&blob);
    EXPECT_TRUE(output.write("hello"));
    EXPECT_EQ(output.position(), 5u);
    EXPECT_TRUE(output.close());

    output.open(&blob);
    EXPECT_EQ(output.position(), 0u);
    EXPECT_TRUE(output.close());
}
