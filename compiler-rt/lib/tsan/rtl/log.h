#ifndef TSAN_LOG_H
#define TSAN_LOG_H

#include "stdio.h"

void log(void* addr, char* msg);

#endif // !TSAN_LOG_H
