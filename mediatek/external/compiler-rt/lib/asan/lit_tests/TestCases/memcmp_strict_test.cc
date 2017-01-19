// RUN: %clangxx_asan -O0 %s -o %t && ASAN_OPTIONS=strict_memcmp=0 %t
// RUN: %clangxx_asan -O0 %s -o %t && ASAN_OPTIONS=strict_memcmp=1 not %t 2>&1 | FileCheck %s
// Default to strict_memcmp=1.
// RUN: %clangxx_asan -O0 %s -o %t && not %t 2>&1 | FileCheck %s

#include <stdio.h>
#include <string.h>
int main() {
  char kFoo[] = "foo";
  char kFubar[] = "fubar";
  int res = memcmp(kFoo, kFubar, strlen(kFubar));
  printf("res: %d\n", res);
  // CHECK: AddressSanitizer: stack-buffer-overflow
  return 0;
}
