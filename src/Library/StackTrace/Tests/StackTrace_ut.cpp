#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <string>
#include <thread>

#include "Testing/Unit/UnitTest.h"

#include "Library/StackTrace/StackTrace.h"
#include "Library/StackTrace/StackTraceOnCrash.h"

#include "Utility/Attributes.h"
#include "Utility/String/Format.h"
#include "Utility/String/Split.h"

#ifndef __ANDROID__ // Stack traces are not supported on android.

/**
 * Matches when the frame numbered `index` names `function`. The regexes gtest's own death test matchers take
 * aren't portable - gtest picks between two engines with different grammars depending on the platform - so
 * this walks the lines instead.
 */
MATCHER_P2(HasFrame, index, function, "") {
    std::string prefix = fmt::format("#{} ", index);
    for (std::string_view line : split(std::string_view(arg)).by('\n'))
        if (line.starts_with(prefix) && line.contains(function))
            return true;
    return false;
}

/**
 * Not inlined so that it gets a frame of its own, and not static because windows drops private symbols from a
 * stripped pdb.
 *
 * @return                              Stack trace taken inside this function.
 */
MM_NOINLINE std::string stackTraceMarkerFunction() {
    std::string trace = stackTraceToString();

    // Deriving the result from the trace keeps the call above out of tail position. Clang tail-calls it at
    // -O2 otherwise, and then this frame, which is the one the test looks for, isn't in the trace at all.
    return trace.empty() ? std::string() : trace;
}

MM_NOINLINE int stackTraceCrashingFunction() {
    // Volatile pointer, not pointer to volatile, or the compiler knows it's null and traps instead of
    // faulting. The read feeds the return value because a store on its own is dead code that some compilers
    // drop, and then nothing crashes at all.
    int *volatile nowhere = nullptr;
    *nowhere = 1;
    return *nowhere;
}

/**
 * Calling the pure virtual from the constructor reaches the base vtable before the derived one is installed,
 * which is the one reliable way to end up in the pure call handler.
 */
struct StackTracePureCallBase {
    StackTracePureCallBase() { callPureIndirectly(); }
    virtual void callPure() = 0;

    MM_NOINLINE void callPureIndirectly() {
        // An extra hop the compiler can't fold away. Inlined into the constructor, the call site would have a
        // known dynamic type, and the call devirtualizes into a direct one to a function with no body.
        callPure();
    }
};
struct StackTracePureCallDerived : StackTracePureCallBase {
    virtual void callPure() override {}
};

// Not static because windows drops private symbols from a stripped pdb.
MM_NOINLINE void stackTracePureCallFunction() {
    StackTracePureCallDerived derived;
}

MM_NOINLINE void stackTraceTerminateFunction() {
    volatile int keepFrame = 0;
    std::terminate();
    keepFrame = 1; // Or the noreturn call becomes a jump, and this frame is gone before the handler runs.
}

MM_NOINLINE void stackTraceAbortFunction() {
    volatile int keepFrame = 0;
    std::abort();
    keepFrame = 1; // Same as in the terminate one above.
}
#ifdef _WIN32
MM_NOINLINE void stackTraceInvalidParameterFunction() {
    volatile int keepFrame = 0; // Same tail-call trap as the abort and terminate ones below.
    std::printf(nullptr); // Null format string is the canonical way to trip the invalid parameter handler.
    keepFrame = 1;
}

#endif // _WIN32

MM_NOINLINE int stackTraceNullCallFunction() {
    int (*volatile nowhere)() = nullptr; // Volatile, or the compiler sees the target and emits a trap instead.
    volatile int result = nowhere(); // Using the result keeps this out of tail position, which keeps the frame.
    return result + 1;
}

MM_NOINLINE int stackTraceBadTargetCallFunction() {
    int (*volatile nowhere)() = reinterpret_cast<int (*)()>(static_cast<uintptr_t>(0xdeadbeefdeadULL));
    volatile int result = nowhere(); // Same volatile dance as in the null call above.
    return result + 1;
}

#ifndef _WIN32
/**
 * Overflows the stack. The pad makes each frame big enough to get there fast, and feeding it into the return
 * value keeps the recursion from being folded into a loop.
 *
 * @param depth                         Recursion depth, start at zero.
 * @return                              Never returns, the stack runs out first.
 */
MM_NOINLINE int stackTraceOverflowFunction(int depth) {
    volatile char pad[1024];
    pad[0] = static_cast<char>(depth);
    return pad[0] + stackTraceOverflowFunction(depth + 1);
}
#endif // !_WIN32

UNIT_TEST(StackTrace, FunctionNamesAreResolved) {
    std::string trace = stackTraceMarkerFunction();

    EXPECT_THAT(trace, HasFrame(1, "stackTraceMarkerFunction"));
    EXPECT_CONTAINS(trace, "main");
}

UNIT_TEST(StackTrace, CrashHandlerNamesTheCrashingFunction) {
    // The crash path is the one that matters, and it's the one that breaks silently - a handler that traces
    // the wrong thread, or traces nothing at all, still exits with the right signal.
    EXPECT_DEATH({
        // Gtest wraps test bodies in __try/__except, and a frame-based handler runs before any unhandled
        // exception filter, so on windows ours would never see the access violation below.
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceCrashingFunction();
    }, testing::AllOf(HasFrame(0, "stackTraceCrashingFunction"), testing::HasSubstr("main")));
}

UNIT_TEST(StackTrace, CrashOnAnotherThreadIsTraced) {
    // The handlers are process-wide, but only the thread that installs them gets an alternate signal stack,
    // so this one runs on the worker's own stack. That's enough for anything short of stack exhaustion.
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        std::thread(stackTraceCrashingFunction).join();
    // A worker's stack ends at the thread entry, so main being absent is what says we traced the thread that
    // crashed rather than the one that installed the handlers.
    }, testing::AllOf(HasFrame(0, "stackTraceCrashingFunction"),
                      testing::Not(testing::HasSubstr("main"))));
}

UNIT_TEST(StackTrace, NullFunctionCallIsTraced) {
    // Calling a null pointer faults at address zero, where there's nothing to unwind from. The call pushed its
    // return address first though, and walking on from that names the function that made the call and
    // everything above it.
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceNullCallFunction();
    }, testing::AllOf(HasFrame(0, "stackTraceNullCallFunction"), testing::HasSubstr("main")));
}

UNIT_TEST(StackTrace, BadTargetCallIsTraced) {
    // Guards against detecting a bad call target by comparing the pc to si_addr. On x86-64 a jump to a
    // non-canonical address is a general protection fault with si_addr reported as zero, so that comparison
    // only ever passed the null call, where the two happen to be equal.
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceBadTargetCallFunction();
    }, testing::AllOf(HasFrame(0, "stackTraceBadTargetCallFunction"), testing::HasSubstr("main")));
}

#ifndef _WIN32
UNIT_TEST(StackTrace, StackOverflowIsTraced) {
    // The handlers run on an alternate stack, and this is what checks it. Without one the handler itself
    // faults on the exhausted stack and the crash prints nothing at all.
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceOverflowFunction(0);
    }, HasFrame(0, "stackTraceOverflowFunction"));
}
#endif // !_WIN32

// The reason string is only asserted on windows, where a dedicated CRT hook prints it. On posix these crashes
// all arrive as SIGABRT and go through the signal handler like any other, and what matters is that the trace
// still names the function that started it, several frames below the abort machinery.
#ifdef _WIN32
#   define MM_TEST_CRT_REASON(REASON, FRAME) testing::AllOf(testing::HasSubstr(REASON), testing::HasSubstr(FRAME))
#else
#   define MM_TEST_CRT_REASON(REASON, FRAME) testing::HasSubstr(FRAME)
#endif

UNIT_TEST(StackTrace, AbortIsTraced) {
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceAbortFunction();
    }, MM_TEST_CRT_REASON("abort()", "stackTraceAbortFunction"));
}

UNIT_TEST(StackTrace, TerminateIsTraced) {
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceTerminateFunction();
    }, MM_TEST_CRT_REASON("std::terminate()", "stackTraceTerminateFunction"));
}

UNIT_TEST(StackTrace, PureVirtualCallIsTraced) {
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTracePureCallFunction();
    }, MM_TEST_CRT_REASON("pure virtual function call", "stackTracePureCallFunction"));
}

#ifdef _WIN32
UNIT_TEST(StackTrace, InvalidParameterIsTraced) {
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceInvalidParameterFunction();
    }, testing::AllOf(testing::HasSubstr("invalid parameter passed to a CRT function"),
                      testing::HasSubstr("stackTraceInvalidParameterFunction")));
}
#endif // _WIN32

#endif // !__ANDROID__
