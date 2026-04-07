#pragma once

#include <stdbool.h>

#ifndef FP_ASSERT
  #include <assert.h>
  #define FP_ASSERT(x) assert(x)
#endif

#ifndef FP_UNUSED
  #define FP_UNUSED(x) ((void)(x))
#endif

