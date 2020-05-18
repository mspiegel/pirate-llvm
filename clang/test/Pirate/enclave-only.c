// RUN: %clang -fuse-ld=lld -ffunction-sections -fdata-sections -Xlinker -enclave -Xlinker e -o /dev/null %s
// REQUIRES: system-linux

#pragma pirate enclave declare(e)

void demo(void)
__attribute__((pirate_enclave_only("e")))
{ }

int enclave(void)
__attribute__((pirate_enclave_main("e")))
{
  demo();	
  return 0;
}
