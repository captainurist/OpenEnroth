#include "KeyboardController.h"

#include "Library/Platform/Application/PlatformApplication.h"

KeyboardController::KeyboardController() : PlatformEventFilter({EVENT_KEY_PRESS, EVENT_KEY_RELEASE}) {}

KeyState KeyboardController::keyState(PlatformKey key) const {
    if (key == PlatformKey::KEY_NONE)
        return {};

    KeyState result;
    // If a key was pressed & released, we assume it's down until the end of the frame.
    result.isDown = _isDown[key] || _pressedThisFrame[key];
    result.pressedThisFrame = _pressedThisFrame[key];
    result.millisecondsSincePressed = _currentFrameTimeMs - _pressTimeMs[key];
    return result;
}

void KeyboardController::reset() {
    _isDown.fill(false);
    _pressedThisFrame.fill(false);
    _pressTimeMs.fill(_currentFrameTimeMs);
}

void KeyboardController::processMessages(PlatformEventHandler *eventHandler) {
    _currentFrameTimeMs = application()->platform()->tickCount();
    _pressedThisFrame.fill(false);
    ProxyEventLoop::processMessages(eventHandler);
}

bool KeyboardController::keyPressEvent(const PlatformKeyEvent *event) {
    if (_isDown[event->key])
        return false; // Auto repeat

    _isDown[event->key] = true;
    _pressedThisFrame[event->key] = true;
    _pressTimeMs[event->key] = _currentFrameTimeMs;
    return false;
}

bool KeyboardController::keyReleaseEvent(const PlatformKeyEvent *event) {
    _isDown[event->key] = false;
    return false;
}
