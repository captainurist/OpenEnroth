#pragma once

#include <unordered_map>

#include "Utility/Error/Result.h"
#include "SoundInfo.h"

class SoundList {
 public:
    SoundInfo *soundInfo(SoundId soundId); // TODO(captainurist): should be const

    friend Result<void> deserialize(const Blob &src, SoundList *dst); // In TableSerialization.cpp.

 private:
    std::unordered_map<SoundId, SoundInfo> _mapSounds;
};

extern SoundList *pSoundList;
