// RUN: not %clang -fuse-ld=lld -fdata-sections -Xlinker -enclave -Xlinker e -o /dev/null %s 2>&1 | FileCheck %s
// REQUIRES: system-linux

#pragma pirate enclave declare(e)
#pragma pirate capability declare(mycap)
void function(void) __attribute__((pirate_capability("mycap"))) {}

void function2(void) { function(); }

int enclave(void)
__attribute__((pirate_enclave_main("e")))
{
  function2();
  return 0;
}
// CHECK: error
