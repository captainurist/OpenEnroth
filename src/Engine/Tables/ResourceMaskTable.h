#pragma once

#include <unordered_map>

#include "Engine/Data/ResourceMask.h"

#include "Utility/String/TransparentFunctors.h"

struct ResourceMaskTable {
    std::unordered_map<std::string, ResourceMask, TransparentStringHash, TransparentStringEquals> bitmaps;
    std::unordered_map<std::string, ResourceMask, TransparentStringHash, TransparentStringEquals> icons;
};
MM_DECLARE_JSON_SERIALIZATION_FUNCTIONS(ResourceMaskTable)
