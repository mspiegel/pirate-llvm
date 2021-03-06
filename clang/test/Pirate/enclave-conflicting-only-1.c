// RUN: not %clang -fuse-ld=lld -ffunction-sections -fdata-sections -Xlinker -enclave -Xlinker e1 -o /dev/null %s 2>&1 | FileCheck -v %s
// REQUIRES: system-linux

#pragma pirate enclave declare(e1)
#pragma pirate enclave declare(e2)

void demo(void)
__attribute__((pirate_enclave_only("e2")))
{ }

int enclave(void)
__attribute__((pirate_enclave_main("e1")))
{
  demo();	
  return 0;
}
// CHECK: error: Symbol demo needed, but can be linked on enclave e2 only
