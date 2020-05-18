// RUN: not %clang -fdata-sections -c -o - %s 2>&1 | FileCheck %s
// REQUIRES: system-linux

// CHECK: error: no enclave named e
int aglobal __attribute__((pirate_enclave_only("e")));
