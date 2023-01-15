#ifndef TSAN_LOG_H
#define TSAN_LOG_H

#include "stdio.h"
#include "tsan_rtl.h"
#include "sanitizer_common/sanitizer_common.h"
#include "tsan_defs.h"
#include "tsan_platform.h"
#include "tsan_report.h"
#include "tsan_sync.h"
#include "sanitizer_common/sanitizer_file.h"
#include "sanitizer_common/sanitizer_placement_new.h"
#include "sanitizer_common/sanitizer_report_decorator.h"
#include "sanitizer_common/sanitizer_stacktrace_printer.h"

namespace __tsan {
    void log(char* msg);
    void PrintVectorClock(__tsan::Context* ctx, __tsan::ThreadState* thr);
}

#endif // !TSAN_LOG_H
