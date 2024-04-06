; int test_strength_reduction(int e, int a) {
;   int b = a + 1;
;   int c = b * 30;
;   int d = e * 1;
;   int b = a / 10;
;   int f = b / 16	
;   return c * f;
; }

define dso_local i32 @test_strength_reduction(i32 noundef %0, i32 noundef %1) #0 {
  %3 = add nsw i32 %1, 1
  %4 = mul nsw i32 %3, 30
  %5 = mul nsw i32 %0, 1
  %6 = udiv i32 %1, 10
  %7 = udiv i32 %6, 16
  %8 = mul nsw i32 %4, %7
  ret i32 %8
}
