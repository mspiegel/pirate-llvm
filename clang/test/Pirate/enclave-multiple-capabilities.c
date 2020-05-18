// RUN: %clang -fuse-ld=lld -ffunction-sections -fdata-sections -o /dev/null %s -Xlinker -enclave -Xlinker e
// REQUIRES: system-linux

#pragma pirate capability declare(cap1)
#pragma pirate capability declare(cap2)

#pragma pirate enclave declare(e)
#pragma pirate enclave capability(e,cap1)
#pragma pirate enclave capability(e,cap2)

int aglobal __attribute__((pirate_capability("cap1"), pirate_capbility("cap2")));

int enclave(void)
__attribute__((pirate_enclave_main("e")))
{
	aglobal = 0;
	return 0;
}
