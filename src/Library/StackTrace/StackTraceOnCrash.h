#pragma once

/**
 * Installs crash handlers that dump a stack trace to stderr, then let the crash proceed so that the OS still
 * produces a core dump or a crash report. The handlers are never uninstalled, the process is dying anyway.
 *
 * Construct one in `main` before anything else can crash. The handlers need stack of their own for when the
 * crash is stack exhaustion - an alternate signal stack on posix, a committed stack guarantee on windows -
 * and both are per-thread, so only the thread that constructs this gets one. On a worker thread the handlers
 * are left to run on whatever stack remains. The handlers themselves are process-wide.
 *
 * The handlers are not async-signal-safe, and can't be - symbolizing a trace allocates, reads files and takes
 * locks, so a crash while another thread holds one of those hangs the process instead of killing it. Crashing
 * inside the allocator does the same.
 */
// What the crash handlers do after printing a trace, right before the process dies.
enum class CrashWait {
    CRASH_WAIT_NONE, // Just die.
    CRASH_WAIT_FOR_INPUT, // Wait for a key press. For a process that owns its console window, which closes with it.
};
using enum CrashWait;

class StackTraceOnCrash {
 public:
    /**
     * @param wait                      What to do after printing a crash trace.
     */
    explicit StackTraceOnCrash(CrashWait wait = CRASH_WAIT_NONE);
};
