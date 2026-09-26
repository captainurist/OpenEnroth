// flagtest KERNEL THREADS SECONDS
// Sets x86 flags with a known outcome, runs some instructions that don't touch flags, then reads ZF, PF and CF back
// and counts every read that disagrees with what the compare must have produced. x86_64 only.
//
// Kernels:
//   oe       the exact shape from Colorf::toHsvColorf: ucomiss equal values, xorps, jne, jnp
//   eq0      ucomiss equal values, read flags immediately (expect ZF=1 PF=0 CF=0)
//   eq8      ucomiss equal values, 8 addps between compare and read
//   eq64     ucomiss equal values, 64 addps between compare and read
//   lt64     ucomiss a < b, 64 addps between (expect ZF=0 PF=0 CF=1)
//   int64    cmp eax, eax, 64 addps between (expect ZF=1 PF=1 CF=0)
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static atomic_int g_stop = 0;
static const char *g_kernel = "oe";
static atomic_long g_total = 0, g_badZ = 0, g_badP = 0, g_badC = 0, g_badBranch = 0;

#define PAD8 "addps %%xmm2, %%xmm3\n addps %%xmm2, %%xmm3\n addps %%xmm2, %%xmm3\n addps %%xmm2, %%xmm3\n" \
             "addps %%xmm2, %%xmm3\n addps %%xmm2, %%xmm3\n addps %%xmm2, %%xmm3\n addps %%xmm2, %%xmm3\n"
#define PAD64 PAD8 PAD8 PAD8 PAD8 PAD8 PAD8 PAD8 PAD8

static inline unsigned flagsUcomiss0(float a, float b) {
    unsigned char z, p, c;
    __asm__ volatile("ucomiss %4, %3\n setz %0\n setp %1\n setc %2\n"
                     : "=r"(z), "=r"(p), "=r"(c) : "x"(a), "x"(b) : "cc");
    return z | (p << 1) | (c << 2);
}

static inline unsigned flagsUcomiss8(float a, float b) {
    unsigned char z, p, c;
    __asm__ volatile("ucomiss %4, %3\n" PAD8 "setz %0\n setp %1\n setc %2\n"
                     : "=r"(z), "=r"(p), "=r"(c) : "x"(a), "x"(b) : "cc", "xmm2", "xmm3");
    return z | (p << 1) | (c << 2);
}

static inline unsigned flagsUcomiss64(float a, float b) {
    unsigned char z, p, c;
    __asm__ volatile("ucomiss %4, %3\n" PAD64 "setz %0\n setp %1\n setc %2\n"
                     : "=r"(z), "=r"(p), "=r"(c) : "x"(a), "x"(b) : "cc", "xmm2", "xmm3");
    return z | (p << 1) | (c << 2);
}

static inline unsigned flagsInt64(unsigned v) {
    unsigned char z, p, c;
    __asm__ volatile("cmp %3, %3\n" PAD64 "setz %0\n setp %1\n setc %2\n"
                     : "=r"(z), "=r"(p), "=r"(c) : "r"(v) : "cc", "xmm2", "xmm3");
    return z | (p << 1) | (c << 2);
}

// Returns 1 when the jne/jnp pair took the "not equal" path for equal inputs.
static inline int oeShape(float a, float b) {
    int bad;
    __asm__ volatile("ucomiss %2, %1\n"
                     "xorps %2, %2\n"
                     "jne 1f\n"
                     "jnp 2f\n"
                     "1: movl $1, %0\n"
                     "jmp 3f\n"
                     "2: movl $0, %0\n"
                     "3:\n"
                     : "=r"(bad), "+x"(a), "+x"(b) : : "cc");
    return bad;
}

static void count(unsigned got, unsigned want, long *z, long *p, long *c) {
    if ((got & 1) != (want & 1)) (*z)++;
    if ((got & 2) != (want & 2)) (*p)++;
    if ((got & 4) != (want & 4)) (*c)++;
}

static void *worker(void *arg) {
    unsigned seed = (unsigned) (uintptr_t) arg * 2654435761u + 7;
    long total = 0, z = 0, p = 0, c = 0, branch = 0;
    while (!atomic_load_explicit(&g_stop, memory_order_relaxed)) {
        for (int i = 0; i < 1000000; i++) {
            seed = seed * 1103515245u + 12345u;
            float x = (float) (seed >> 16) / 256.0f;
            if (strcmp(g_kernel, "oe") == 0) {
                branch += oeShape(x, x);
            } else if (strcmp(g_kernel, "eq0") == 0) {
                count(flagsUcomiss0(x, x), 1, &z, &p, &c);
            } else if (strcmp(g_kernel, "eq8") == 0) {
                count(flagsUcomiss8(x, x), 1, &z, &p, &c);
            } else if (strcmp(g_kernel, "eq64") == 0) {
                count(flagsUcomiss64(x, x), 1, &z, &p, &c);
            } else if (strcmp(g_kernel, "lt64") == 0) {
                count(flagsUcomiss64(x, x + 1.0f), 4, &z, &p, &c);
            } else {
                count(flagsInt64(seed), 1 | 2, &z, &p, &c);
            }
        }
        total += 1000000;
    }
    atomic_fetch_add(&g_total, total);
    atomic_fetch_add(&g_badZ, z);
    atomic_fetch_add(&g_badP, p);
    atomic_fetch_add(&g_badC, c);
    atomic_fetch_add(&g_badBranch, branch);
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "usage: flagtest KERNEL THREADS SECONDS\n");
        return 2;
    }
    g_kernel = argv[1];
    int threads = atoi(argv[2]), seconds = atoi(argv[3]);
    pthread_t t[64];
    for (int i = 0; i < threads; i++)
        pthread_create(&t[i], NULL, worker, (void *) (uintptr_t) (i + 1));
    sleep(seconds);
    atomic_store(&g_stop, 1);
    for (int i = 0; i < threads; i++)
        pthread_join(t[i], NULL);
    long bad = atomic_load(&g_badZ) + atomic_load(&g_badP) + atomic_load(&g_badC) + atomic_load(&g_badBranch);
    printf("RESULT kernel=%s threads=%d seconds=%d iterations=%ld badZF=%ld badPF=%ld badCF=%ld badBranch=%ld\n",
           g_kernel, threads, seconds, atomic_load(&g_total), atomic_load(&g_badZ), atomic_load(&g_badP),
           atomic_load(&g_badC), atomic_load(&g_badBranch));
    return bad ? 3 : 0;
}
