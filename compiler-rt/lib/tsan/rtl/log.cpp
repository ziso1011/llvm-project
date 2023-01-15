#include "log.h"

void log(char* msg) {
  FILE *log_file = fopen("tsan.log", "a");
  // fprintf(log_file, "Address %p: %s\n", addr, msg);
  fprintf(log_file, "%s", msg);
  fclose(log_file);
}

void PrintVectorClock(__tsan::Context* ctx, __tsan::ThreadState* thr) {
    unsigned long nthread, nlive;
    ctx->thread_registry.GetNumberOfThreads(&nthread, &nlive);
    __sanitizer::Printf("Vector clock: [ ");
    for (int i = 0; i < nthread; ++i) {
        __sanitizer::Printf("%d ", (int)thr->clock.clk_[i]);
    }
    __sanitizer::Printf("]\n");
}