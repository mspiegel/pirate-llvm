// RUN: not %clang -fuse-ld=lld -fdata-sections -Xlinker -enclave -Xlinker e -o /dev/null %s 2>&1 | FileCheck %s
// REQUIRES: system-linux

#pragma pirate enclave declare(e)
#pragma pirate capability declare(mycap)
int global __attribute__((pirate_capability("mycap"))) {}

int function(void) { global = 0; }

int enclave(void)
__attribute__((pirate_enclave_main("e")))
{
  function();
  return 0;
}
// CHECK: error
