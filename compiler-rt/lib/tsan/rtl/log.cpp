#include "log.h"

void log(void* addr, char* msg) {
  FILE *log_file = fopen("tsan.log", "a");
  fprintf(log_file, "Address %p: %s\n", addr, msg);
  fclose(log_file);
}