; int test_multi_inst(int e, int a) {
;   int b = a + 1;    // -> multi: optmization on d = b - 1
;   int c = b * 3;    // -> multi: optmization on c = c / 3
;                     // -> c = b << 2; c = c - b
;   int d = b - 1;    // -> multi: d = a
;   int f = 3 / c;    // not optimized
;   c = c / 3;        // -> multi: c = b
;   d = 3 * f;        // -> d = f << 2; d = d - f
;   d = d + c         // not optimized
;   return f * d;
; }

define dso_local i32 @test_multi_inst(i32 noundef %0, i32 noundef %1) #0 {
  %3 = add nsw i32 %1, 1
  %4 = mul nsw i32 %3, 3
  %5 = sub i32 %3, 1
  %6 = udiv i32 3, %4
  %7 = udiv i32 %4, 3
  %8 = mul nsw i32 3, %6
  %9  = add nsw %8 %7
  %10 = mul nsw i32 %6, %9
  ret i32 %9
}
