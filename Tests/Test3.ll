; int test_multi_inst(int e, int a) {
;   int b = a + 1;
;   int c = b * 3;
;   int d = b - 1;
;   int f = 3 / c;
;   c = c / 3;
;   d = 3 * f;
;   return f * d;
; }

define dso_local i32 @test_multi_inst(i32 noundef %0, i32 noundef %1) #0 {
  %3 = add nsw i32 %1, 1
  %4 = mul nsw i32 %3, 3
  %5 = sub i32 %3, 1
  %6 = udiv i32 3, %4
  %7 = udiv i32 %4, 3
  %8 = mul nsw i32 3, %6
  %9 = mul nsw i32 %6, %8
  ret i32 %9
}
