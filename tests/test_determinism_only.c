#include <stdio.h>

int test_determinism(void);

int main(void) {
  int fail = test_determinism();
  if (fail) {
    fprintf(stderr, "Determinism test failed.\n");
    return 1;
  }
  printf("Determinism test passed.\n");
  return 0;
}
