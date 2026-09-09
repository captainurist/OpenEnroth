#pragma once

#include <memory>
#include <functional>
#include <utility>

#include "Library/Platform/Interface/PlatformOpenGLOptions.h"

#include "PlatformIntrospection.h"
#include "PlatformComponentStorage.h"

class FilteringEventHandler;
class ApplicationProxy;

// Probably should be a container of:
// - root objects (systems)?
// - components.
//
// root objects are either:
// - proxyable (Platform)
// - filter-chainable (FilteringEventHandler)
//
// components are:
// - proxies
// - filters
// - can replace root objects altogether in install/uninstallNotify.
//
// How to implement this properly? Components will need to have a common base, ugh.

// installGlobal(T, id)
// removeGlobal(T, id)
// global<T>(id)
//
// installComponent(T, id)
// removeComponent(T, id)
// component<T>(id)
//
// Global should specialize application_proxy<T>
// OR application_filter<T>
//
// Then just do std::reflect::bases_of to find a tree of bases, and go over registered proxy types to match. HAH!
// Need C++26.
//
// Wait. Components will need to explicitly state WHAT they are overriding. B/c can override one RNG or the other one.
// Or one FS or the other one.
//
// UGH. But, wait. RNG and FS use non-default ids. So for non-default IDS you need to do the magic in installNotify.
// Otherwise the magic happens auto-magically.

/**
 * This class ties together everything in platform for a particular use case of an application with a single window.
 *
 * The proper way to use it is by creating components that implement the pieces of functionality that you need,
 * and then installing them into the `PlatformApplication`.
 *
 * Supported base classes for components are:
 * - All kinds of platform proxies;
 * - `PlatformEventFilter`;
 * - `PlatformApplicationAware`.
 *
 * Components can have multiple bases, this is handled automatically. If you want to use private bases, befriend
 * `PlatformIntrospection`.
 *
 * The components you install are inserted into the proxy and event filter chains exactly in the order of installation.
 * The ordering is usually important, so it makes sense to install everything in a single place. If this model doesn't
 * work for you (e.g. you have two components that implement several proxies that need to be ordered differently w.r.t.
 * each other), then you'll have to split your component into several parts.
 */
class PlatformApplication {
 public:
    explicit PlatformApplication(Platform *platform);
    ~PlatformApplication();

    void initializeOpenGLContext(const PlatformOpenGLOptions &options);
    void initializeOpenGLContext(std::unique_ptr<PlatformOpenGLContext> context);

    Platform *platform();
    PlatformEventLoop *eventLoop();
    PlatformWindow *window();
    PlatformOpenGLContext *openGLContext();
    PlatformEventHandler *eventHandler();

    template<class T>
    T *installComponent(std::unique_ptr<T> component) {
        PlatformIntrospection::visit(component.get(), [this] (auto *specificComponent) {
            installComponentInternal(specificComponent);
        });

        std::function<void()> cleanup = [this, component = component.get()] {
            PlatformIntrospection::visit(component, [this] (auto *specificComponent) {
                removeComponentInternal(specificComponent);
            });
        };

        return _components.insert(std::move(component), std::move(cleanup));
    }

    template<class T>
    std::unique_ptr<T> removeComponent() {
        return _components.remove<T>();
    }

    const PlatformComponentStorage *components() const {
        return &_components;
    }

    template<class T>
    T *component() const {
        return _components.require<T>();
    }

    void processMessages(int count = -1);
    void waitForMessages();

 private:
    friend class PlatformApplicationAware;

    void installComponentInternal(ProxyPlatform *platform);
    void removeComponentInternal(ProxyPlatform *platform);
    void installComponentInternal(ProxyEventLoop *eventLoop);
    void removeComponentInternal(ProxyEventLoop *eventLoop);
    void installComponentInternal(ProxyWindow *window);
    void removeComponentInternal(ProxyWindow *window);
    void installComponentInternal(ProxyOpenGLContext *openGLContext);
    void removeComponentInternal(ProxyOpenGLContext *openGLContext);
    void installComponentInternal(PlatformEventFilter *eventFilter);
    void removeComponentInternal(PlatformEventFilter *eventFilter);
    void installComponentInternal(PlatformApplicationAware *aware);
    void removeComponentInternal(PlatformApplicationAware *aware);

 private:
    Platform *_platform;
    std::unique_ptr<PlatformEventLoop> _eventLoop;
    std::unique_ptr<PlatformWindow> _window;
    std::unique_ptr<PlatformOpenGLContext> _openGLContext;
    std::unique_ptr<FilteringEventHandler> _eventHandler;
    std::unique_ptr<ApplicationProxy> _rootProxy;
    PlatformComponentStorage _components;
};
