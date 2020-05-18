// RUN: not %clang -fuse-ld=lld -fdata-sections -ffunction-sections -Xlinker -enclave -Xlinker e -o /dev/null %s 2>&1 | FileCheck %s
// REQUIRES: system-linux

#pragma pirate enclave declare(e)
#pragma pirate capability declare(micro)
#pragma pirate capability declare(mega,micro)
#pragma pirate enclave capability(e,micro)

// CHECK: error: Symbol aglobal needed, but has unmet requirement mega
int aglobal __attribute__((pirate_capability("mega")));

int enclave(void)
__attribute__((pirate_enclave_main("e")))
{
  return aglobal;
}
