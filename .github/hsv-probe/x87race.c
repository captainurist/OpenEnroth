// x87race CHECKERS NOISE SECONDS NOISE_KIND
// Checker threads call fmodf(h + 360, 360) twice per value and flag a result outside [0, 360) or two calls that
// disagree. Noise threads run concurrently: kind "x87" does long double arithmetic, kind "fmod" calls fmodf, kind
// "sse" does float arithmetic only. Exit code 3 means at least one bad result.
#include <math.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static atomic_long g_bad = 0;
static atomic_long g_total = 0;
static atomic_int g_stop = 0;
static const char *g_noiseKind = "x87";

static unsigned bitsOf(float f) {
    unsigned result;
    memcpy(&result, &f, 4);
    return result;
}

static void *checker(void *arg) {
    unsigned seed = (unsigned) (uintptr_t) arg * 2654435761u + 1;
    long local = 0;
    while (!atomic_load_explicit(&g_stop, memory_order_relaxed)) {
        for (int i = 0; i < 100000; i++) {
            seed = seed * 1103515245u + 12345u;
            volatile float h = (float) (seed >> 8) * (360.0f / 16777216.0f);
            float r1 = fmodf(h + 360.0f, 360.0f);
            float r2 = fmodf(h + 360.0f, 360.0f);
            if (!(r1 >= 0.0f && r1 < 360.0f) || bitsOf(r1) != bitsOf(r2)) {
                long b = atomic_fetch_add(&g_bad, 1);
                if (b < 20)
                    fprintf(stderr, "BAD h=%.9g(%08x) r1=%.9g(%08x) r2=%.9g(%08x)\n", h, bitsOf(h), r1, bitsOf(r1), r2, bitsOf(r2));
            }
        }
        local += 100000;
    }
    atomic_fetch_add(&g_total, local);
    return NULL;
}

static void *noise(void *arg) {
    (void) arg;
    if (strcmp(g_noiseKind, "x87") == 0) {
        volatile long double x = 1.0L;
        while (!atomic_load_explicit(&g_stop, memory_order_relaxed))
            for (int i = 0; i < 100000; i++)
                x = x * 1.0000001L + 0.5L - x / 3.0L;
    } else if (strcmp(g_noiseKind, "fmod") == 0) {
        volatile float x = 1.0f;
        while (!atomic_load_explicit(&g_stop, memory_order_relaxed))
            for (int i = 0; i < 100000; i++)
                x = fmodf(x * 7.3f + 1000.0f, 997.0f);
    } else {
        volatile float x = 1.0f;
        while (!atomic_load_explicit(&g_stop, memory_order_relaxed))
            for (int i = 0; i < 100000; i++)
                x = x * 1.0000001f + 0.5f - x / 3.0f;
    }
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "usage: x87race CHECKERS NOISE SECONDS [x87|fmod|sse]\n");
        return 2;
    }
    int checkers = atoi(argv[1]), noises = atoi(argv[2]), seconds = atoi(argv[3]);
    if (argc >= 5)
        g_noiseKind = argv[4];
    pthread_t threads[64];
    int n = 0;
    for (int i = 0; i < checkers; i++)
        pthread_create(&threads[n++], NULL, checker, (void *) (uintptr_t) (i + 1));
    for (int i = 0; i < noises; i++)
        pthread_create(&threads[n++], NULL, noise, NULL);
    sleep(seconds);
    atomic_store(&g_stop, 1);
    for (int i = 0; i < n; i++)
        pthread_join(threads[i], NULL);
    printf("RESULT checkers=%d noise=%d kind=%s seconds=%d calls=%ld bad=%ld\n", checkers, noises, g_noiseKind, seconds,
           atomic_load(&g_total) * 2, atomic_load(&g_bad));
    return atomic_load(&g_bad) ? 3 : 0;
}
