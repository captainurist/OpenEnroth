#pragma once

#include <cstddef>

struct Color;
struct HsvColorf;

// Probe-only diagnostics for the HsvColorf range assert that fires under Rosetta. Not for merging.

extern int g_hsvProbePass;

[[noreturn]] void hsvProbeReport(const char *what, const float *values, int count);

[[noreturn]] void hsvProbeVerify(Color input, size_t index, float xs, float xv, const HsvColorf &hsv, const HsvColorf &adj);

inline bool hsvProbeInRange(float h, float s, float v, float a) {
    return h >= 0.0f && h <= 360.0f && s >= 0.0f && s <= 1.0f && v >= 0.0f && v <= 1.0f && a >= 0.0f && a <= 1.0f;
}
