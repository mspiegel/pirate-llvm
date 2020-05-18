// RUN: %clang -fdata-sections -c -o - %s | llvm-readelf --pirate-info | FileCheck %s
// REQUIRES: system-linux

#pragma pirate capability declare(mycap)
int aglobal __attribute__((pirate_capability("mycap")));
// CHECK: Pirate Requirements:
// CHECK:   Symbol                Enclave                 Capabilities
// CHECK:   aglobal (2)           0                       mycap (1)
// CHECK: Pirate Capabilities
// CHECK:         Name                  Parent
// CHECK:   0                           0
// CHECK:   1     mycap                 0
