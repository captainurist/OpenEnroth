#pragma once

#include <memory>

#include "AudioDataSource.h"


// OK design time.
//
//
// MediaPlatform
//  virtual std::unique_ptr<MediaStream> open(Blob, OpenOpts) -- because we can.
//
// MediaStream
//  virtual std::unique_ptr<MediaPacket> next() -- demuxing, decoding, transforming into requested format --- all here
//
// MediaPacket
//  type
//  PcmData OR VideoData --- literally pcm OR pixels
//
//
// On top of it:
// DecodingThread:
//  calls next(), puts in queue of limited size.
//
// AudioPlaybackThread --- not really needed. Will do everything in Player::update.
//
// Player:
//  calls open()
//  creates DecodingThread
//  loops & pushes packets into the right places.
//
// AudioPlayer:
//  calls open()
//  creates DecodingStream
//  creates

class IAudioTrack {
 public:
    IAudioTrack() {}
    virtual ~IAudioTrack() {}

    virtual bool Open(PAudioDataSource data_source) = 0;
    virtual bool IsValid() = 0;

    virtual bool Play() = 0;
    virtual bool Stop() = 0;
    virtual bool Pause() = 0;
    virtual bool Resume() = 0;
    virtual bool SetVolume(float volume) = 0;
    virtual float GetVolume() = 0;
};
typedef std::shared_ptr<IAudioTrack> PAudioTrack;

