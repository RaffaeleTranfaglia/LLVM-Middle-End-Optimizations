; int test_algebraic_identity(int e, int a) {
;   int b = a + 0;
;   int c = 0 - b;
;   b = e / 1;
;   c = b + a;
;   return c * b;
; }

define dso_local i32 @test_algebraic_identity(i32 noundef %0, i32 noundef %1) #0 {
  %3 = add nsw i32 %1, 0
  %4 = sub i32 0, %3
  %5 = udiv i32 %0, 1
  %6 = add i32 %5, %1
  %7 = mul nsw i32 %6, %5
  ret i32 %7
}
