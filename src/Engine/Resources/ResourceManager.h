#pragma once

#include <string_view>

#include "Engine/Tables/ResourceMaskTable.h"

#include "Library/Image/Image.h"
#include "Library/Lod/LodReader.h"

#include "Utility/Memory/Blob.h"


// Games.lod moves here.
//
// LOD accessors move here.
//
// LOD caches die --- no need to cache raw image data.
//
// We have one TextureCache --- that's also super lazy. literally strings for shit we don't want evicted.
//
// std::shared_ptr<Texture> for textures.
//
// gc() in cache - if (unique()) then reset()
//
// No caching in ResourceManager - only masking, so that we don't hardcode it.
//
// Implementation plan:
// - Move all data access to ResourceManager. All colorkey logic into a separate json file.
// - Create global TextureCache.
// - Drop Lod caches.

struct LodImage;
struct Palette;

/**
 * This class provides access to everything in `/data` folder.
 */
class ResourceManager {
 public:
    ResourceManager();
    ~ResourceManager();

    void open();

    Blob event(std::string_view filename);

    RgbaImage bitmap(std::string_view filename);
    RgbaImage generated(std::string_view filename);
    RgbaImage icon(std::string_view filename);
    RgbaImage sprite(std::string_view filename); // TODO(captainurist): GrayscaleImage

 private:
    static RgbaImage makeRgbaImageWithInterpolatedMask(GrayscaleImageView indexedImage, const Palette &palette, int maskIndex);
    static int maskIndexFor(const LodImage &image, const ResourceMask &mask);
    static void applyMask(const ResourceMask &mask, LodImage *image, int *maskIndex = nullptr);
    static void applyMask(const ResourceMask &mask, RgbaImage *image);

 private:
    ResourceMaskTable _masks;
    LodReader _eventsLodReader;
    LodReader _bitmapsLodReader;
    LodReader _iconsLodReader;
    LodReader _spriteLodReader;
};
