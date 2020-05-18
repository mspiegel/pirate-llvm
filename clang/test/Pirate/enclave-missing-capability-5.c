// RUN: not %clang -fuse-ld=lld -fdata-sections -Xlinker -enclave -Xlinker e -o /dev/null %s 2>&1 | FileCheck %s
// REQUIRES: system-linux

#pragma pirate capability declare(cap1)
#pragma pirate capability declare(cap2)
#pragma pirate enclave declare(e)
#pragma pirate enclave capability(e,cap1)

int global __attribute__((pirate_capability("cap1"), pirate_capability("cap2"));

int enclave(void)
__attribute__((pirate_enclave_main("e")))
{
  global = 0;
  return 0;
}
// CHECK: error
