#include "core/fp_arena.h"

#include <string.h>

static size_t fp_align_up(size_t x, size_t a) {
  size_t m = a - 1;
  return (x + m) & ~m;
}

void fp_arena_init(FpArena* a, void* mem, size_t cap) {
  a->base = (uint8_t*)mem;
  a->cap = cap;
  a->off = 0;
}

void fp_arena_reset(FpArena* a) {
  a->off = 0;
}

void* fp_arena_alloc(FpArena* a, size_t size, size_t align) {
  if (align < 1) align = 1;
  size_t at = fp_align_up(a->off, align);
  if (at + size > a->cap) return NULL;
  void* p = a->base + at;
  a->off = at + size;
  memset(p, 0, size);
  return p;
}

