#include <cstdint>
#include <utility>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/Binary/BlobSerialization.h"
#include "Library/Binary/CommonSerialization.h"

#include "Utility/Streams/BlobOutputStream.h"
#include "Utility/Streams/BlobInputStream.h"

UNIT_TEST(Binary, GarbageSizeThrowsInsteadOfBadAlloc) {
    // Craft a stream with a garbage uint32_t size prefix that claims more elements than the stream contains.
    // This should throw a descriptive serialization error, not std::bad_alloc.
    Blob blob;
    BlobOutputStream out(&blob);
    uint32_t garbageSize = 2'000'000'000; // Claims 2B ints, but stream only has 4+4=8 bytes total.
    int oneElement = 42;
    out.write(&garbageSize, sizeof(garbageSize));
    out.write(&oneElement, sizeof(oneElement));
    out.close();

    BlobInputStream input(std::move(blob));
    std::vector<int> dst;
    EXPECT_THROW_MESSAGE(deserialize(input, &dst), "expected");
}

UNIT_TEST(Binary, TryDeserializeReportsShortBlob) {
    // A blob that's too short to hold the whole value comes back as an error, not an exception.
    uint64_t dst = 0;
    Result<void> result = tryDeserialize(Blob::fromString("xx"), &dst);
    ASSERT_FALSE(result);
    EXPECT_THAT(result.error().message(), testing::HasSubstr("expected 8 bytes"));
}

UNIT_TEST(Binary, TryDeserializeReportsLeftoverData) {
    uint32_t dst = 0;
    Result<void> result = tryDeserialize(Blob::fromString("12345678"), &dst);
    ASSERT_FALSE(result);
    EXPECT_THAT(result.error().message(), testing::HasSubstr("bytes left"));
}
