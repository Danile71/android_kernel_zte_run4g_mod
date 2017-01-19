// RUN: %clangxx_asan  %s -o %t

// Regular run.
// RUN: not %t 2> %t.out
// RUN: FileCheck %s --check-prefix=CHECK-ERROR < %t.out

// Good log_path.
// RUN: rm -f %t.log.*
// RUN: ASAN_OPTIONS=log_path=%t.log not %t 2> %t.out
// RUN: FileCheck %s --check-prefix=CHECK-ERROR < %t.log.*

// Invalid log_path.
// RUN: ASAN_OPTIONS=log_path=/INVALID not %t 2> %t.out
// RUN: FileCheck %s --check-prefix=CHECK-INVALID < %t.out

// Too long log_path.
// RUN: ASAN_OPTIONS=log_path=`for((i=0;i<10000;i++)); do echo -n $i; done` \
// RUN:   not %t 2> %t.out
// RUN: FileCheck %s --check-prefix=CHECK-LONG < %t.out

// Run w/o errors should not produce any log.
// RUN: rm -f %t.log.*
// RUN: ASAN_OPTIONS=log_path=%t.log  %t ARG ARG ARG
// RUN: not cat %t.log.*


#include <stdlib.h>
#include <string.h>
int main(int argc, char **argv) {
  if (argc > 2) return 0;
  char *x = (char*)malloc(10);
  memset(x, 0, 10);
  int res = x[argc * 10];  // BOOOM
  free(x);
  return res;
}
// CHECK-ERROR: ERROR: AddressSanitizer
// CHECK-INVALID: ERROR: Can't open file: /INVALID
// CHECK-LONG: ERROR: Path is too long: 01234
