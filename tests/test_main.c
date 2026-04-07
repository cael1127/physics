#include <stdio.h>
#include <stdlib.h>

int test_collision(void);
int test_determinism(void);
int test_simd(void);

int main(void) {
  int fail = 0;
  fail |= test_collision();
  fail |= test_determinism();
  fail |= test_simd();
  if (fail) {
    fprintf(stderr, "Tests failed.\n");
    return 1;
  }
  printf("All tests passed.\n");
  return 0;
}

