// RUN: %clang -fuse-ld=lld -ffunction-sections -fdata-sections -Xlinker -enclave -Xlinker e -o /dev/null %s
// REQUIRES: system-linux

#pragma pirate enclave declare(e)

#define ONLY __attribute__((pirate_enclave_only("e")))

int restricted_global ONLY;

void restricted_function1(void) ONLY {}

void restricted_function2(void) ONLY {}

void inherited_restriction1(void) { restricted_function1(); }

void inherited_restriction2(void) { restricted_global = 0; }

int enclave(void)
__attribute__((pirate_enclave_main("e")))
{
  restricted_global = 0;
  restricted_function1();
  restricted_function2();
  inherited_restriction1();
  inherited_restriction2();
  return 0;
}
