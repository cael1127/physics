#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct FpArena {
  uint8_t* base;
  size_t cap;
  size_t off;
} FpArena;

void fp_arena_init(FpArena* a, void* mem, size_t cap);
void fp_arena_reset(FpArena* a);
void* fp_arena_alloc(FpArena* a, size_t size, size_t align);

