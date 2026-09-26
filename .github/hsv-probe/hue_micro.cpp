// Standalone HsvColorf probe, built against the repo's own Colorf.cpp and HsvColorf.cpp.
//
// hue_micro startup      One pass shaped like PaletteManager::load, 999 palettes of 256 colours, then exit.
// hue_micro full K       K passes over all 2^24 colours at 16 saturation/lightness pairs, in one process.
//
// Out-of-range values are checked explicitly and printed with their bits before the conversion that would
// assert, so a hit says what the value was. Exit code 3 means a hit.
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "Library/Color/Color.h"
#include "Library/Color/Colorf.h"
#include "Library/Color/HsvColorf.h"

static long g_bad = 0;

static unsigned bitsOf(float f) {
    unsigned result;
    std::memcpy(&result, &f, 4);
    return result;
}

static bool inRange(const HsvColorf &c) {
    return c.h >= 0.0f && c.h <= 360.0f && c.s >= 0.0f && c.s <= 1.0f && c.v >= 0.0f && c.v <= 1.0f && c.a >= 0.0f && c.a <= 1.0f;
}

static void report(const char *stage, Color in, float xs, float xv, const HsvColorf &c) {
    g_bad++;
    std::fprintf(stderr, "HIT %s rgb(%d,%d,%d) xs=%g xv=%g h=%.9g(%08x) s=%.9g(%08x) v=%.9g(%08x) a=%.9g(%08x)\n",
                 stage, in.r, in.g, in.b, xs, xv, c.h, bitsOf(c.h), c.s, bitsOf(c.s), c.v, bitsOf(c.v), c.a, bitsOf(c.a));
}

static void convert(Color in, float xs, float xv) {
    HsvColorf hsv = in.toHsvColorf();
    if (!inRange(hsv)) {
        report("toHsvColorf", in, xs, xv, hsv);
        return;
    }
    HsvColorf adj = hsv.adjusted(0, xs, xv);
    if (!inRange(adj)) {
        report("adjusted", in, xs, xv, adj);
        return;
    }
    volatile Color out = adj.toColor();
    (void) out;
}

int main(int argc, char **argv) {
    if (argc >= 2 && std::strcmp(argv[1], "startup") == 0) {
        for (int i = 0; i < 999 * 256; i++)
            convert(Color((i * 37) & 255, (i * 101) & 255, (i * 211) & 255, 255), 0.65f, 1.1f);
    } else if (argc >= 3 && std::strcmp(argv[1], "full") == 0) {
        const float xss[] = {0.65f, 1.0f, 0.0f, 2.0f};
        const float xvs[] = {1.1f, 1.0f, 0.0f, 2.0f};
        int passes = std::atoi(argv[2]);
        for (int pass = 0; pass < passes; pass++)
            for (int rgb = 0; rgb < (1 << 24); rgb++)
                for (float xs : xss)
                    for (float xv : xvs)
                        convert(Color(rgb & 255, (rgb >> 8) & 255, (rgb >> 16) & 255, 255), xs, xv);
        std::printf("full passes=%d hits=%ld\n", passes, g_bad);
    } else {
        std::fprintf(stderr, "usage: hue_micro startup | hue_micro full K\n");
        return 2;
    }
    return g_bad ? 3 : 0;
}
