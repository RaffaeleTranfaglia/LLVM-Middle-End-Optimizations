; ModuleID = 'TEST/loop_fus_ex1_nomem.bc'
source_filename = "TEST/loop_fus_ex1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32 noundef %0, ptr noundef %1, ptr noundef %2) {
  %4 = zext i32 %0 to i64
  %5 = call ptr @llvm.stacksave()
  %6 = alloca i32, i64 %4, align 16
  %7 = zext i32 %0 to i64
  %8 = alloca i32, i64 %7, align 16
  br label %9

9:                                                ; preds = %23, %3
  %.01 = phi i32 [ 0, %3 ], [ %24, %23 ]
  %10 = icmp slt i32 %.01, %0
  br i1 %10, label %11, label %25

11:                                               ; preds = %9
  %12 = sext i32 %.01 to i64
  %13 = getelementptr inbounds i32, ptr %1, i64 %12
  %14 = load i32, ptr %13, align 4
  %15 = sdiv i32 1, %14
  %16 = sext i32 %.01 to i64
  %17 = getelementptr inbounds i32, ptr %2, i64 %16
  %18 = load i32, ptr %17, align 4
  %19 = mul nsw i32 %15, %18
  %20 = add nsw i32 %.01, 1
  %21 = sext i32 %20 to i64
  %22 = getelementptr inbounds i32, ptr %6, i64 %21
  store i32 %19, ptr %22, align 4
  br label %23

23:                                               ; preds = %11
  %24 = add nsw i32 %.01, 1
  br label %9, !llvm.loop !6

25:                                               ; preds = %9
  br label %26

26:                                               ; preds = %38, %25
  %.0 = phi i32 [ 0, %25 ], [ %39, %38 ]
  %27 = icmp slt i32 %.0, %0
  br i1 %27, label %28, label %40

28:                                               ; preds = %26
  %29 = sext i32 %.0 to i64
  %30 = getelementptr inbounds i32, ptr %6, i64 %29
  %31 = load i32, ptr %30, align 4
  %32 = sext i32 %.0 to i64
  %33 = getelementptr inbounds i32, ptr %2, i64 %32
  %34 = load i32, ptr %33, align 4
  %35 = add nsw i32 %31, %34
  %36 = sext i32 %.0 to i64
  %37 = getelementptr inbounds i32, ptr %8, i64 %36
  store i32 %35, ptr %37, align 4
  br label %38

38:                                               ; preds = %28
  %39 = add nsw i32 %.0, 1
  br label %26, !llvm.loop !8

40:                                               ; preds = %26
  call void @llvm.stackrestore(ptr %5)
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare ptr @llvm.stacksave() #0

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore(ptr) #0

attributes #0 = { nocallback nofree nosync nounwind willreturn }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 17.0.6 (https://github.com/Glixes/LLVM_middle_end.git 4a7534547dac66ae3b99e37aa760138030286260)"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
!8 = distinct !{!8, !7}
