#pragma once

#include <string>
#include <functional>

#include "Library/Color/Color.h"
#include "Library/Image/Image.h"
#include "Library/Image/Palette.h"

class LodSpriteCache;
class LodTextureCache;
class LodReader;

class ImageLoader {
 public:
    virtual ~ImageLoader() = default;
    const std::string &GetResourceName() const { return this->resource_name; }

    virtual bool Load(RgbaImage *rgbaImage) = 0;

 protected:
    std::string resource_name;
};

class Buff_LOD_Loader : public ImageLoader {
 public:
    inline Buff_LOD_Loader(LodTextureCache *lod, std::string_view filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;

 protected:
    LodTextureCache *lod;
};

class PCX_Loader : public ImageLoader {
 protected:
    bool InternalLoad(const Blob &data, RgbaImage *rgbaImage);
};

class PCX_LOD_Raw_Loader : public PCX_Loader {
 public:
    inline PCX_LOD_Raw_Loader(LodReader *lod, std::string_view filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;

 protected:
    LodReader *lod;
};

class Bitmaps_LOD_Loader : public ImageLoader {
 public:
    inline Bitmaps_LOD_Loader(LodTextureCache *lod, std::string_view filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;

 protected:
    LodTextureCache *lod;
};

class Bitmaps_GEN_Loader : public ImageLoader {
 public:
    explicit inline Bitmaps_GEN_Loader(std::string_view filename) {
        this->resource_name = filename;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;
};

class Sprites_LOD_Loader : public ImageLoader {
 public:
    inline Sprites_LOD_Loader(LodSpriteCache *lod, std::string_view filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;

 protected:
    LodSpriteCache *lod;
};
