// RUN: %clang -fdata-sections -c -o - %s | llvm-readelf --pirate-info | FileCheck %s
// REQUIRES: system-linux

#pragma pirate enclave declare(e)
#pragma pirate capability declare(mycap)
#pragma pirate enclave capability(e,mycap)
int aglobal __attribute__((pirate_capability("mycap")));

int enclave(void)
__attribute__((pirate_enclave_main("e")))
{
  return aglobal;
}
// CHECK: Pirate Enclaves:
// CHECK:         Name                  Main symbol             Capabilities
// CHECK:   0     0                     0                       0
// CHECK:   1     e                     enclave (4)             1: mycap (1)
// CHECK: Pirate Requirements:
// CHECK:   Symbol                Enclave                 Capabilities
// CHECK:   aglobal (3)           0                       mycap (1)
// CHECK: Pirate Capabilities
// CHECK:         Name                  Parent
// CHECK:   0                           0
// CHECK:   1     mycap                 0

