#include "Engine/Graphics/Image.h"

#include <cassert>
#include <utility>
#include <memory>
#include <string>

#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/AssetsManager.h"

GraphicsImage::GraphicsImage() = default;

GraphicsImage::~GraphicsImage() {
    releaseRenderId();
}

std::unique_ptr<GraphicsImage> GraphicsImage::Create(RgbaImage image) {
    std::unique_ptr<GraphicsImage> result = std::make_unique<GraphicsImage>();
    result->_initialized = true;
    result->_rgba = std::move(image);
    result->_renderId = render->CreateTexture(result->_rgba);
    return result;
}

std::unique_ptr<GraphicsImage> GraphicsImage::Create(Sizei size) {
    assert(size.w > 0 && size.h > 0);
    return Create(RgbaImage::solid(size.w, size.h, Color()));
}

std::unique_ptr<GraphicsImage> GraphicsImage::Create(std::unique_ptr<ImageLoader> loader) {
    std::unique_ptr<GraphicsImage> result = std::make_unique<GraphicsImage>();
    result->_name = loader->GetResourceName();
    result->_loader = std::move(loader);
    return result;
}

ssize_t GraphicsImage::width() {
    return rgba().width();
}

ssize_t GraphicsImage::height() {
    return rgba().height();
}

Sizei GraphicsImage::size() {
    return rgba().size();
}

RgbaImage &GraphicsImage::rgba() {
    initialize();
    return _rgba;
}

const std::string &GraphicsImage::name() {
    return _name;
}

[[nodiscard]] TextureRenderId GraphicsImage::renderId() {
    if (!_renderId) {
        initialize();
        _renderId = render->CreateTexture(_rgba);
    }

    return _renderId;
}

void GraphicsImage::releaseRenderId() {
    if (!_renderId)
        return;

    if (render)
        render->DeleteTexture(_renderId);
    _renderId = TextureRenderId();
}

bool GraphicsImage::initialize() {
    if (_initialized)
        return true;

    assert(_loader);
    _initialized = _loader->Load(&_rgba);
    // TODO(captainurist): _initialized == false happens, investigate

    return _initialized;
}
