// RUN: %clang -ffunction-sections -fdata-sections -S -emit-llvm -c -o - %s | FileCheck %s
// REQUIRES: system-linux

#pragma pirate enclave declare(e)

typedef int int_resource __attribute__((pirate_resource_type("normal_int")));

int_resource my_resource
__attribute__((
  pirate_resource("res_name", "e")
));

int_resource my_resource2
__attribute__((
  pirate_resource("res_name2", "e"),
  pirate_resource_param("Param", "Value")
));

// CHECK: @my_resource = common dso_local global i32 0, align 4
// CHECK: @.str = private unnamed_addr constant [9 x i8] c"res_name\00", align 1
// CHECK: @my_resource.const = private unnamed_addr constant <{ i8*, i8*, { i8*, i8* }*, i64 }> <{ i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i64 0, i64 0), i8* bitcast (i32* @my_resource to i8*), { i8*, i8* }* null, i64 0 }>, section ".pirate.res.normal_int.e", align 16
// CHECK: @my_resource2 = common dso_local global i32 0, align 4
// CHECK: @.str.1 = private unnamed_addr constant [10 x i8] c"res_name2\00", align 1
// CHECK: @.str.2 = private unnamed_addr constant [6 x i8] c"Param\00", align 1
// CHECK: @.str.3 = private unnamed_addr constant [6 x i8] c"Value\00", align 1
// CHECK: @my_resource2.const = private unnamed_addr constant [1 x { i8*, i8* }] [{ i8*, i8* } { i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.2, i64 0, i64 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.3, i64 0, i64 0) }], align 8
// CHECK: @my_resource2.const.4 = private unnamed_addr constant <{ i8*, i8*, { i8*, i8* }*, i64 }> <{ i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str.1, i64 0, i64 0), i8* bitcast (i32* @my_resource2 to i8*), { i8*, i8* }* getelementptr inbounds ([1 x { i8*, i8* }], [1 x { i8*, i8* }]* @my_resource2.const, i64 0, i64 0), i64 1 }>, section ".pirate.res.normal_int.e", align 16
