#ifndef TSAN_LOG_H
#define TSAN_LOG_H

#include "stdio.h"
#include "tsan_rtl.h"
#include "sanitizer_common/sanitizer_common.h"
#include "tsan_defs.h"
#include "tsan_platform.h"
#include "tsan_report.h"
#include "tsan_sync.h"
#include "tsan_interface.h"
#include "sanitizer_common/sanitizer_file.h"
#include "sanitizer_common/sanitizer_placement_new.h"
#include "sanitizer_common/sanitizer_report_decorator.h"
#include "sanitizer_common/sanitizer_stacktrace_printer.h"

namespace __tsan {
    void log(char* msg);
    void PrintVectorClock(__tsan::Context* ctx, __tsan::ThreadState* thr);
}

#define ENABLE_TSAN_DEFAULT_OUTPUT
#define LOG_THREAD_ON_READ
#define LOG_THREAD_ON_WRITE
#define LOG_MUTEX_LOCK_UNLOCK
#define LOG_THREAD_JOIN
#define LOG_THREAD_FORK
#define LOG_THREAD_EPOCH
// #define LOG_THREAD_FINISHED
// #define LOG_MUTEX_EPOCH_INCREMENTS
// #define LOG_MUTEX_ACTIONS
// #define LOG_VECTOR_CLOCK

#endif // !TSAN_LOG_H
