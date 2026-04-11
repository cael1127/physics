#include <stdio.h>
#include <stdlib.h>

int test_collision(void);
int test_determinism(void);
int test_simd(void);
int test_fluid(void);
int test_aero(void);
int test_scene(void);

int main(void) {
  int fail = 0;
  fail |= test_collision();
  fail |= test_determinism();
  fail |= test_simd();
  fail |= test_fluid();
  fail |= test_aero();
  fail |= test_scene();
  if (fail) {
    fprintf(stderr, "Tests failed.\n");
    return 1;
  }
  printf("All tests passed.\n");
  return 0;
}

