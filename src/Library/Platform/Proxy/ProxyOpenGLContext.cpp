#include "ProxyOpenGLContext.h"

ProxyOpenGLContext::ProxyOpenGLContext(PlatformOpenGLContext *base) : ProxyBase<PlatformOpenGLContext>(base) {}

bool ProxyOpenGLContext::bind() {
    return nonNullBase()->bind();
}

bool ProxyOpenGLContext::unbind() {
    return nonNullBase()->unbind();
}

void ProxyOpenGLContext::startOverlayFrame() {
    nonNullBase()->startOverlayFrame();
}

void ProxyOpenGLContext::swapBuffers() {
    nonNullBase()->swapBuffers();
}

void *ProxyOpenGLContext::getProcAddress(const char *name) {
    return nonNullBase()->getProcAddress(name);
}
