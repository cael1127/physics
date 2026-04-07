#pragma once

#include <stdarg.h>

typedef enum FpLogLevel {
  FP_LOG_INFO = 0,
  FP_LOG_WARN = 1,
  FP_LOG_ERROR = 2
} FpLogLevel;

void fp_log(FpLogLevel level, const char* fmt, ...);
void fp_vlog(FpLogLevel level, const char* fmt, va_list args);

