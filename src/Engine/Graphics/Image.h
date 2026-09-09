#pragma once

#include <string>
#include <memory>

#include "Engine/Graphics/Renderer/TextureRenderId.h"

#include "Library/Geometry/Size.h"
#include "Library/Image/Image.h"
#include "Library/Image/Palette.h"

#include "Utility/Types.h"

// Renderer API:
// - Take Texture*. Store renderId - but only until end of frame.
// - ReleaseTexture releases after end of frame.
// => airtight API. User can't see that we're not drawing on every call.
// => UpdateTexture needs to go.
//
// Texture API:
// - Just use constructors. No reason for std::unique_ptr factory methods. Memory management is external.
//
// AssetManager API:
// - Returns std::shared_ptr.
// - Smart release.
//   - Milliseconds since last access.
//   - Evict after 30s.
//   - No need for smarter logic (tracking eviction-reloads) - will just prolong the time item textures are in mem.
//
// Can drop UpdateTexture first in a separate commit.
//
// Then use ctors & dtors in Texture.
//
// Then use shared_ptrs. Gonna have a shitload of conflicts.

class ImageLoader;

class GraphicsImage {
 public:
   GraphicsImage();
    ~GraphicsImage();

    static std::unique_ptr<GraphicsImage> Create(RgbaImage image);
    static std::unique_ptr<GraphicsImage> Create(Sizei size);
    static std::unique_ptr<GraphicsImage> Create(std::unique_ptr<ImageLoader> loader);

    ssize_t width();
    ssize_t height();
    Sizei size();

    RgbaImage &rgba();

    const std::string &name();

    [[nodiscard]] TextureRenderId renderId();
    void releaseRenderId();

 private:
    bool initialize();

 private:
    bool _initialized = false;
    std::string _name;
    std::unique_ptr<ImageLoader> _loader;
    RgbaImage _rgba;
    TextureRenderId _renderId;
};
