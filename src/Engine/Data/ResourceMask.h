#pragma once

#include "Library/Json/JsonFwd.h"
#include "Library/Serialization/SerializationFwd.h"
#include "Library/Color/Color.h"

enum MaskMode {
    MASK_DEFAULT,   // Use the `zeroIsTransparent` field, that's the default.
    MASK_NONE,      // No transparency.
    MASK_ZERO,      // Take transparency from palette entry #0, ignore `zeroIsTransparent` field.
    MASK_COLOR,     // Use provided transparency mask color.
};
MM_DECLARE_SERIALIZATION_FUNCTIONS(MaskMode);

struct ResourceMask {
    MaskMode mode = MASK_DEFAULT;
    Color color;
};
MM_DECLARE_SERIALIZATION_FUNCTIONS(ResourceMask)
MM_DECLARE_JSON_SERIALIZATION_FUNCTIONS(ResourceMask)
