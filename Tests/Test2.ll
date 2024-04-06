; int test_strength_reduction(int e, int a) {
;   int b = a + 1;    // not optimized
;   int c = b * 30;   // -> c = b << 5; c1 = b * 2; c = c - c1;
;                     // -> c = b << 5; c1 = b << 1; c = c - c1;
;   int d = e * 1;    // -> d = e; -> deleted
;   int f = a / 10;   // not optimized
;   int g = c / 16	  // -> c >> 4
;   int h = b + d;    // -> h = b + e
;   int i = f + g;    // -> i = f + g
;   return h * 1;
; }

define dso_local i32 @test_strength_reduction(i32 noundef %0, i32 noundef %1) #0 {
  %3 = add nsw i32 %1, 1
  %4 = mul nsw i32 %3, 30
  %5 = mul nsw i32 %0, 1
  %6 = udiv i32 %1, 10
  %7 = udiv i32 %4, 16
  %8 = add i32 %3, %5
  %9 = add i32 %6, %7
  %10 = mul nsw i32 %8, %9
  ret i32 %10
}
