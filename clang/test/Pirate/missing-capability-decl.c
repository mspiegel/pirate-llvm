// RUN: not %clang -fdata-sections -c -o - %s 2>&1 | FileCheck %s
// REQUIRES: system-linux

// CHECK: error: no capability named mycap
int aglobal __attribute__((pirate_capability("mycap")));
