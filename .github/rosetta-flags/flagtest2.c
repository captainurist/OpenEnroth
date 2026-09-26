// flagtest2 SECONDS KERNEL [KERNEL...]    one thread per KERNEL argument
// flagtest2 dump                           write this binary's executable Rosetta AOT mapping to flagtest2_aot_*.bin
//
// Every kernel sets x86 flags with a known outcome, optionally runs flag-neutral SSE padding, then reads RFLAGS with
// pushfq and compares CF, PF, AF, ZF, SF and OF with the expected value. Bad reads are counted per kernel and the
// distinct bad RFLAGS values are kept, so a hit says which bits went wrong.
//
//   eq0 eq8 eq64 eq512    ucomiss of equal values, 0/8/64/512 addps between compare and read    want ZF
//   lt64                  ucomiss a < b, 64 addps                                               want CF
//   un64                  ucomiss with a NaN, 64 addps                                          want ZF PF CF
//   int64                 cmp eax, eax, 64 addps                                                want ZF PF
//   oe                    the Colorf::toHsvColorf shape: ucomiss, xorps, jne, jnp               counts wrong branches
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <libproc.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>

#define CF 0x001
#define PF 0x004
#define AF 0x010
#define ZF 0x040
#define SF 0x080
#define OF 0x800
#define MASK (CF | PF | AF | ZF | SF | OF)

#define PAD8 "addps %%xmm2, %%xmm3\n addps %%xmm2, %%xmm3\n addps %%xmm2, %%xmm3\n addps %%xmm2, %%xmm3\n" \
             "addps %%xmm2, %%xmm3\n addps %%xmm2, %%xmm3\n addps %%xmm2, %%xmm3\n addps %%xmm2, %%xmm3\n"
#define PAD64 PAD8 PAD8 PAD8 PAD8 PAD8 PAD8 PAD8 PAD8
#define PAD512 PAD64 PAD64 PAD64 PAD64 PAD64 PAD64 PAD64 PAD64

#define UCOMISS_KERNEL(NAME, PAD)                                                                                      \
    static inline unsigned long NAME(float a, float b) {                                                               \
        unsigned long f;                                                                                               \
        __asm__ volatile("ucomiss %2, %1\n" PAD "pushfq\n pop %0\n" : "=r"(f) : "x"(a), "x"(b) : "cc", "xmm2", "xmm3"); \
        return f & MASK;                                                                                               \
    }

UCOMISS_KERNEL(ucomiss0, "")
UCOMISS_KERNEL(ucomiss8, PAD8)
UCOMISS_KERNEL(ucomiss64, PAD64)
UCOMISS_KERNEL(ucomiss512, PAD512)

static inline unsigned long cmp64(unsigned v) {
    unsigned long f;
    __asm__ volatile("cmp %1, %1\n" PAD64 "pushfq\n pop %0\n" : "=r"(f) : "r"(v) : "cc", "xmm2", "xmm3");
    return f & MASK;
}

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

enum { K_EQ0, K_EQ8, K_EQ64, K_EQ512, K_LT64, K_UN64, K_INT64, K_OE, K_COUNT };
static const char *kNames[K_COUNT] = {"eq0", "eq8", "eq64", "eq512", "lt64", "un64", "int64", "oe"};
static const unsigned long kWant[K_COUNT] = {ZF, ZF, ZF, ZF, CF, ZF | PF | CF, ZF | PF, 0};

#define MAX_VALUES 8
typedef struct {
    int kernel;
    int id;
    long total, bad;
    unsigned long values[MAX_VALUES];
    long valueCounts[MAX_VALUES];
    long firstBadAt;
} Worker;

static atomic_int g_stop = 0;

static void record(Worker *w, unsigned long got) {
    if (w->bad == 0)
        w->firstBadAt = w->total;
    w->bad++;
    for (int i = 0; i < MAX_VALUES; i++) {
        if (w->valueCounts[i] && w->values[i] == got) {
            w->valueCounts[i]++;
            return;
        }
        if (!w->valueCounts[i]) {
            w->values[i] = got;
            w->valueCounts[i] = 1;
            return;
        }
    }
}

static void *run(void *arg) {
    Worker *w = arg;
    unsigned seed = (unsigned) w->id * 2654435761u + 7;
    unsigned long want = kWant[w->kernel];
    while (!atomic_load_explicit(&g_stop, memory_order_relaxed)) {
        for (int i = 0; i < 100000; i++) {
            seed = seed * 1103515245u + 12345u;
            float x = (float) (seed >> 16) / 256.0f;
            unsigned long got;
            switch (w->kernel) {
            case K_EQ0: got = ucomiss0(x, x); break;
            case K_EQ8: got = ucomiss8(x, x); break;
            case K_EQ64: got = ucomiss64(x, x); break;
            case K_EQ512: got = ucomiss512(x, x); break;
            case K_LT64: got = ucomiss64(x, x + 1.0f); break;
            case K_UN64: got = ucomiss64(x, __builtin_nanf("")); break;
            case K_INT64: got = cmp64(seed); break;
            default: got = oeShape(x, x) ? 1 : 0; want = 0; break;
            }
            if (got != want)
                record(w, got);
        }
        w->total += 100000;
    }
    return NULL;
}

static void dumpAot(void) {
    mach_vm_address_t addr = 0;
    mach_vm_size_t size;
    natural_t depth = 0;
    for (;;) {
        struct vm_region_submap_info_64 info;
        mach_msg_type_number_t cnt = VM_REGION_SUBMAP_INFO_COUNT_64;
        if (mach_vm_region_recurse(mach_task_self(), &addr, &size, &depth, (vm_region_recurse_info_t) &info, &cnt))
            break;
        char path[4096] = {0};
        proc_regionfilename(getpid(), addr, path, sizeof(path));
        if (strstr(path, "flagtest2.aot")) {
            char name[128];
            snprintf(name, sizeof(name), "flagtest2_aot_off%llx_prot%d.bin", (unsigned long long) info.offset, info.protection);
            FILE *f = fopen(name, "wb");
            fwrite((void *) addr, 1, size, f);
            fclose(f);
            printf("AOTDUMP %s size=%llu path=%s\n", name, (unsigned long long) size, path);
        }
        addr += size;
    }
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "dump") == 0) {
        dumpAot();
        return 0;
    }
    if (argc < 3) {
        fprintf(stderr, "usage: flagtest2 SECONDS KERNEL [KERNEL...] | flagtest2 dump\n");
        return 2;
    }
    int seconds = atoi(argv[1]), threads = argc - 2;
    Worker *workers = calloc(threads, sizeof(Worker));
    pthread_t *t = calloc(threads, sizeof(pthread_t));
    for (int i = 0; i < threads; i++) {
        workers[i].id = i + 1 + getpid() * 64;
        workers[i].kernel = -1;
        for (int k = 0; k < K_COUNT; k++)
            if (strcmp(argv[i + 2], kNames[k]) == 0)
                workers[i].kernel = k;
        if (workers[i].kernel < 0) {
            fprintf(stderr, "unknown kernel %s\n", argv[i + 2]);
            return 2;
        }
    }
    for (int i = 0; i < threads; i++)
        pthread_create(&t[i], NULL, run, &workers[i]);
    sleep(seconds);
    atomic_store(&g_stop, 1);
    long bad = 0;
    for (int i = 0; i < threads; i++)
        pthread_join(t[i], NULL);
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    printf("RUN pid=%d seconds=%d threads=%d nvcsw=%ld nivcsw=%ld\n", getpid(), seconds, threads, ru.ru_nvcsw, ru.ru_nivcsw);
    for (int i = 0; i < threads; i++) {
        Worker *w = &workers[i];
        bad += w->bad;
        printf("RESULT kernel=%s thread=%d iterations=%ld bad=%ld", kNames[w->kernel], i, w->total, w->bad);
        if (w->bad) {
            printf(" first_bad_at=%ld want=%03lx got=", w->firstBadAt, kWant[w->kernel]);
            for (int j = 0; j < MAX_VALUES && w->valueCounts[j]; j++)
                printf("%s%03lx:%ld", j ? "," : "", w->values[j], w->valueCounts[j]);
        }
        printf("\n");
    }
    return bad ? 3 : 0;
}
