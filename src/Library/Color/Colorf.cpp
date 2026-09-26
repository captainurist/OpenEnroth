#include "Colorf.h"

#include <cassert>
#include <algorithm>

#include "HsvColorf.h"
#include "HsvProbe.h"

#define HSV_PROBE(cond, what, base, count) do { if (__builtin_expect(!(cond), 0)) hsvProbeReport(what, base, count); } while (0)

[[nodiscard]] HsvColorf Colorf::toHsvColorf() const {
    HSV_PROBE(r >= 0.0f && r <= 1.0f, "toHsvColorf.r", &r, 4);
    HSV_PROBE(g >= 0.0f && g <= 1.0f, "toHsvColorf.g", &r, 4);
    HSV_PROBE(b >= 0.0f && b <= 1.0f, "toHsvColorf.b", &r, 4);
    HSV_PROBE(a >= 0.0f && a <= 1.0f, "toHsvColorf.a", &r, 4);

    HsvColorf result;
    result.a = a;

    float max = std::max({r, g, b});
    float min = std::min({r, g, b});
    float delta = max - min;

    // value
    result.v = max;

    // saturation
    if (max == 0.0) {  // r=g=b=0
        result.s = 0.0;
    } else {
        result.s = delta / max;
    }

    float hcalc = 0.0f;

    // hue
    if (max == min) {
        hcalc = 0.0f;
    } else if (max == r) {
        hcalc = (g - b) / delta; // yellow and mag
    } else if (max == g) {
        hcalc = (b - r) / delta + 2.0;    // cyan and yellow
    } else {
        hcalc = (r - g) / delta + 4.0;  // mag and cyan
    }

    result.h = hcalc * 60.0f;  // to degree
    if (result.h < 0.0f)
        result.h += 360.0f;

    return result;
}
