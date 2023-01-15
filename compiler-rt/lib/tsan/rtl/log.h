#ifndef TSAN_LOG_H
#define TSAN_LOG_H

#include "stdio.h"
#include "tsan_platform.h"
#include "tsan_sync.h"

void log(void* addr, char* msg);



#endif // !TSAN_LOG_H
