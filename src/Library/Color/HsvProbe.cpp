#include "HsvProbe.h"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#if defined(__x86_64__)
#include <xmmintrin.h>
#endif

#if defined(__APPLE__)
#include <execinfo.h>
#include <mach/mach.h>
#include <pthread.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#endif

#include "Color.h"
#include "Colorf.h"
#include "HsvColorf.h"

int g_hsvProbePass = 0;

static const auto g_hsvProbeStart = std::chrono::steady_clock::now();

static unsigned bitsOf(float f) {
    unsigned result;
    std::memcpy(&result, &f, 4);
    return result;
}

static unsigned mxcsr() {
#if defined(__x86_64__)
    return _mm_getcsr();
#else
    return 0;
#endif
}

static void printFloats(const char *label, const float *values, int count) {
    std::fprintf(stderr, "HSVPROBE %s", label);
    for (int i = 0; i < count; i++)
        std::fprintf(stderr, " %.9g(%08x)", values[i], bitsOf(values[i]));
    std::fprintf(stderr, "\n");
}

static void printContext() {
    long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - g_hsvProbeStart).count();
    std::fprintf(stderr, "HSVPROBE context pass=%d ms_since_start=%lld mxcsr=%08x\n", g_hsvProbePass, ms, mxcsr());
#if defined(__APPLE__)
    int translated = -1;
    size_t size = sizeof(translated);
    sysctlbyname("sysctl.proc_translated", &translated, &size, nullptr, 0);

    rusage ru = {};
    getrusage(RUSAGE_SELF, &ru);
    std::fprintf(stderr, "HSVPROBE process translated=%d nvcsw=%ld nivcsw=%ld minflt=%ld majflt=%ld nsignals=%ld\n",
                 translated, ru.ru_nvcsw, ru.ru_nivcsw, ru.ru_minflt, ru.ru_majflt, ru.ru_nsignals);

    thread_act_array_t threads = nullptr;
    mach_msg_type_number_t threadCount = 0;
    if (task_threads(mach_task_self(), &threads, &threadCount) == KERN_SUCCESS) {
        mach_port_t self = mach_thread_self();
        std::fprintf(stderr, "HSVPROBE threads=%u\n", threadCount);
        for (mach_msg_type_number_t i = 0; i < threadCount; i++) {
            thread_basic_info_data_t info = {};
            mach_msg_type_number_t infoCount = THREAD_BASIC_INFO_COUNT;
            thread_info(threads[i], THREAD_BASIC_INFO, reinterpret_cast<thread_info_t>(&info), &infoCount);
            char name[64] = "";
            if (pthread_t pt = pthread_from_mach_thread_np(threads[i]))
                pthread_getname_np(pt, name, sizeof(name));
            std::fprintf(stderr, "HSVPROBE   thread %u%s name='%s' run_state=%d suspend=%d user=%d.%06ds sys=%d.%06ds\n",
                         i, threads[i] == self ? " (self)" : "", name, info.run_state, info.suspend_count,
                         info.user_time.seconds, info.user_time.microseconds, info.system_time.seconds, info.system_time.microseconds);
        }
    }

    void *frames[64];
    int frameCount = backtrace(frames, 64);
    backtrace_symbols_fd(frames, frameCount, 2);
#endif
    std::fflush(stderr);
}

void hsvProbeReport(const char *what, const float *values, int count) {
    unsigned csr = mxcsr();
    std::fprintf(stderr, "HSVPROBE HIT %s at %p mxcsr_at_entry=%08x\n", what, static_cast<const void *>(values), csr);
    printFloats("values", values, count);
    const unsigned char *bytes = reinterpret_cast<const unsigned char *>(values);
    std::fprintf(stderr, "HSVPROBE bytes[-32..+48]");
    for (int i = -32; i < 48; i++)
        std::fprintf(stderr, "%s%02x", i == 0 ? " | " : (i % 4 == 0 ? " " : ""), bytes[i]);
    std::fprintf(stderr, "\n");
    printContext();
    std::abort();
}

void hsvProbeVerify(Color input, size_t index, float xs, float xv, const HsvColorf &hsv, const HsvColorf &adj) {
    unsigned csr = mxcsr();
    std::fprintf(stderr, "HSVPROBE VERIFY index=%zu rgba=%u,%u,%u,%u xs=%.9g(%08x) xv=%.9g(%08x) mxcsr_at_entry=%08x\n",
                 index, input.r, input.g, input.b, input.a, xs, bitsOf(xs), xv, bitsOf(xv), csr);
    const float first[8] = {hsv.h, hsv.s, hsv.v, hsv.a, adj.h, adj.s, adj.v, adj.a};
    printFloats("first hsv+adj", first, 8);
    for (int retry = 0; retry < 5; retry++) {
        Colorf f = input.toColorf();
        HsvColorf hsv2 = f.toHsvColorf();
        HsvColorf adj2 = hsv2.adjusted(0, xs, xv);
        const float again[12] = {f.r, f.g, f.b, f.a, hsv2.h, hsv2.s, hsv2.v, hsv2.a, adj2.h, adj2.s, adj2.v, adj2.a};
        printFloats("retry rgbaf+hsv+adj", again, 12);
    }
    printContext();
    std::abort();
}
