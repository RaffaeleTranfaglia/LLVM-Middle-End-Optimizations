define dso_local i32 @test(i32 noundef %0, i32 noundef %1) #0 {
  %3 = add nsw i32 2, 0
  %4 = mul nsw i32 %3, 16
  %5 = shl i32 %0, 1
  %6 = udiv i32 %5, 4
  %7 = mul nsw i32 %4, %6
  %8 = add nsw i32 %3, 0
  %9 = mul nsw i32 %7, 1
  %10 = add nsw i32 %8, %9
  ret i32 %10
}
