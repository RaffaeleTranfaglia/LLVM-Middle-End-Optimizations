; ModuleID = './TEST/Loop_test_nomem.bc'
source_filename = "./TEST/Loop_test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [25 x i8] c"%d,%d,%d,%d,%d,%d,%d,%d\0A\00", align 1

define dso_local void @foo(i32 noundef %0, i32 noundef %1) {
  br label %3

3:                                                ; preds = %17, %2
  %.05 = phi i32 [ 0, %2 ], [ %21, %17 ]
  %.04 = phi i32 [ 0, %2 ], [ %19, %17 ]
  %.03 = phi i32 [ 0, %2 ], [ %18, %17 ]
  %.01 = phi i32 [ 9, %2 ], [ %.1, %17 ]
  %.0 = phi i32 [ %1, %2 ], [ %4, %17 ]
  %4 = add nsw i32 %.0, 1
  %5 = add nsw i32 %0, 3
  %6 = add nsw i32 %0, 7
  %7 = icmp slt i32 %4, 5
  br i1 %7, label %8, label %11

8:                                                ; preds = %3
  %9 = add nsw i32 %.01, 2
  %10 = add nsw i32 %0, 3
  br label %17

11:                                               ; preds = %3
  %12 = sub nsw i32 %.01, 1
  %13 = add nsw i32 %0, 4
  %14 = icmp sge i32 %4, 10
  br i1 %14, label %15, label %16

15:                                               ; preds = %11
  br label %22

16:                                               ; preds = %11
  br label %17

17:                                               ; preds = %16, %8
  %.02 = phi i32 [ %10, %8 ], [ %13, %16 ]
  %.1 = phi i32 [ %9, %8 ], [ %12, %16 ]
  %18 = add nsw i32 %5, 7
  %19 = add nsw i32 %.02, 2
  %20 = add nsw i32 %0, 7
  %21 = add nsw i32 %6, 5
  br label %3

22:                                               ; preds = %15
  %23 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %12, i32 noundef %13, i32 noundef %.03, i32 noundef %.04, i32 noundef %6, i32 noundef %.05, i32 noundef %5, i32 noundef %4)
  ret void
}

declare i32 @printf(ptr noundef, ...)

define dso_local i32 @main() {
  call void @foo(i32 noundef 0, i32 noundef 4)
  call void @foo(i32 noundef 0, i32 noundef 12)
  ret i32 0
}

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 17.0.6 (https://github.com/Glixes/LLVM_middle_end.git 4df89be706dc6fd8853a2e501b3bb76c99981871)"}
