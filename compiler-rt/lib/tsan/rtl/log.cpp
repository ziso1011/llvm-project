#include "log.h"

void log(char* msg) {
  FILE *log_file = fopen("tsan.log", "a");
  // fprintf(log_file, "Address %p: %s\n", addr, msg);
  fprintf(log_file, "%s", msg);
  fclose(log_file);
}