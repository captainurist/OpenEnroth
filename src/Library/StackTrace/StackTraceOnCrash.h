#pragma once

/**
 * Installs crash handlers that dump a stack trace to stderr, then let the crash proceed so that the OS still
 * produces a core dump or a crash report. The handlers are never uninstalled, the process is dying anyway.
 *
 * Construct one in `main` before anything else can crash. On POSIX the handlers run on an alternate stack so
 * that they also work when the crash is stack exhaustion. That stack is per-thread and only the thread that
 * constructs this gets one, so on a worker thread the handlers are left to run on whatever stack remains.
 * The handlers themselves are process-wide. The windows ones don't set up any stack to run on, so a stack
 * overflow there faults again inside the handler instead of printing.
 *
 * The handlers are not async-signal-safe, and can't be - symbolizing a trace allocates, reads files and takes
 * locks, so a crash while another thread holds one of those hangs the process instead of killing it. Crashing
 * inside the allocator does the same.
 */
class StackTraceOnCrash {
 public:
    StackTraceOnCrash();
};
