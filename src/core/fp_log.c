#include "core/fp_log.h"

#include <stdio.h>

static const char* fp_log_prefix(FpLogLevel level) {
  switch (level) {
    case FP_LOG_INFO: return "[info] ";
    case FP_LOG_WARN: return "[warn] ";
    case FP_LOG_ERROR: return "[error]";
    default: return "[log]  ";
  }
}

void fp_vlog(FpLogLevel level, const char* fmt, va_list args) {
  fputs(fp_log_prefix(level), stderr);
  vfprintf(stderr, fmt, args);
  fputc('\n', stderr);
}

void fp_log(FpLogLevel level, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  fp_vlog(level, fmt, args);
  va_end(args);
}

