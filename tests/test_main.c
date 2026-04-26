#include <stdio.h>
#include <stdlib.h>

int test_collision(void);
int test_determinism(void);
int test_simd(void);
int test_fluid(void);
int test_aero(void);
int test_scene(void);
int test_raycast(void);
int test_queries(void);
int test_constraints(void);
int test_replay(void);
int test_revolute(void);

int main(void) {
  int fail = 0;
  fail |= test_collision();
  fail |= test_determinism();
  fail |= test_simd();
  fail |= test_fluid();
  fail |= test_aero();
  fail |= test_scene();
  fail |= test_raycast();
  fail |= test_queries();
  fail |= test_constraints();
  fail |= test_replay();
  fail |= test_revolute();
  if (fail) {
    fprintf(stderr, "Tests failed.\n");
    return 1;
  }
  printf("All tests passed.\n");
  return 0;
}

