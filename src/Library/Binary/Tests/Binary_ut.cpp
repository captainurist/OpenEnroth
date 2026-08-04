#include <cstdint>
#include <utility>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/Binary/CommonSerialization.h"
#include "Library/Binary/BlobSerialization.h"

#include "Utility/Streams/BlobOutputStream.h"
#include "Utility/Streams/BlobInputStream.h"

UNIT_TEST(Binary, GarbageSizeFailsInsteadOfBadAlloc) {
    // Craft a stream with a garbage uint32_t size prefix that claims more elements than the stream contains.
    // This should produce a descriptive serialization error, not std::bad_alloc.
    Blob blob;
    BlobOutputStream out(&blob);
    uint32_t garbageSize = 2'000'000'000; // Claims 2B ints, but stream only has 4+4=8 bytes total.
    int oneElement = 42;
    out.write(&garbageSize, sizeof(garbageSize));
    out.write(&oneElement, sizeof(oneElement));
    out.close();

    BlobInputStream input(std::move(blob));
    std::vector<int> dst;
    deserialize(input, &dst);
    ASSERT_TRUE(input.failed());
    EXPECT_THAT(input.error().message(), testing::HasSubstr("expected"));
    EXPECT_TRUE(dst.empty());
}

UNIT_TEST(Binary, TryDeserializeReportsShortBlob) {
    // A blob that's too short to hold the whole value comes back as an error, with the display path included.
    uint64_t dst = 0;
    Result<void> result = tryDeserialize(Blob::fromString("xx").withDisplayPath("broken.bin"), &dst);
    ASSERT_FALSE(result);
    EXPECT_THAT(result.error().message(), testing::HasSubstr("broken.bin"));
    EXPECT_EQ(dst, 0x7878u); // The 2 bytes that were there, plus a zero-filled tail. Never uninitialized memory.
}

UNIT_TEST(Binary, TryDeserializeReportsLeftoverData) {
    uint32_t dst = 0;
    Result<void> result = tryDeserialize(Blob::fromString("12345678").withDisplayPath("broken.bin"), &dst);
    ASSERT_FALSE(result);
    EXPECT_THAT(result.error().message(), testing::HasSubstr("bytes left"));
}
