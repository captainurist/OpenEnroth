#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Library/Compression/Compression.h"

#include "Utility/Error/Result.h"
#include "Utility/Memory/Blob.h"

static Blob makeCorruptZlibBlob(const Blob &source) {
    // Compress, then corrupt the last 4 bytes (Adler-32 checksum).
    Blob compressed = zlib::compress(source);
    Blob corrupted = Blob::copy(compressed.data(), compressed.size());
    auto *bytes = static_cast<uint8_t *>(const_cast<void *>(corrupted.data()));
    bytes[corrupted.size() - 1] ^= 0xFF; // Flip bits in the last byte.
    bytes[corrupted.size() - 2] ^= 0xFF;
    return corrupted;
}

UNIT_TEST(Compression, UncompressBestEffort_CorruptChecksum) {
    // Data that compresses and decompresses cleanly.
    std::string original(1000, 'A');
    Blob source = Blob::fromString(original);

    // Normal uncompress works.
    Blob compressed = zlib::compress(source);
    Result<Blob> decompressed = zlib::uncompress(compressed, source.size());
    ASSERT_TRUE(decompressed);
    EXPECT_EQ(decompressed->size(), source.size());

    // Corrupt the checksum — strict uncompress fails, best-effort recovers.
    Blob corrupted = makeCorruptZlibBlob(source);
    Result<Blob> failed = zlib::uncompress(corrupted, source.size());
    ASSERT_FALSE(failed);
    EXPECT_THAT(failed.error().message(), testing::HasSubstr("Decompression error"));

    Blob recovered = zlib::uncompressBestEffort(corrupted, source.size());
    ASSERT_EQ(recovered.size(), source.size());
    EXPECT_EQ(std::string_view(static_cast<const char *>(recovered.data()), recovered.size()), original);
}

UNIT_TEST(Compression, UncompressBestEffort_EmptyReturnsEmpty) {
    EXPECT_FALSE(zlib::uncompressBestEffort(Blob()));
}
