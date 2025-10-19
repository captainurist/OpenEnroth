#pragma once

#include "Library/Platform/Application/PlatformApplicationAware.h"
#include "Library/Platform/Filters/PlatformEventFilter.h"
#include "Library/Platform/Proxy/ProxyEventLoop.h"

#include "Utility/IndexedArray.h"

struct KeyState {
   bool isDown = false;
   bool pressedThisFrame = false;
   int64_t millisecondsSincePressed = 0;
};

class KeyboardController : public PlatformEventFilter, public ProxyEventLoop, public PlatformApplicationAware {
public:
   KeyboardController();

   [[nodiscard]] KeyState keyState(PlatformKey key) const;

   void reset();

private:
   virtual void processMessages(PlatformEventHandler *eventHandler) override;
   virtual bool keyPressEvent(const PlatformKeyEvent *event) override;
   virtual bool keyReleaseEvent(const PlatformKeyEvent *event) override;

private:
   IndexedArray<bool, PlatformKey::KEY_FIRST, PlatformKey::KEY_LAST> _isDown = {{}};
   IndexedArray<bool, PlatformKey::KEY_FIRST, PlatformKey::KEY_LAST> _pressedThisFrame = {{}};
   IndexedArray<int64_t, PlatformKey::KEY_FIRST, PlatformKey::KEY_LAST> _pressTimeMs = {{}};
   int64_t _currentFrameTimeMs = 0;
};
