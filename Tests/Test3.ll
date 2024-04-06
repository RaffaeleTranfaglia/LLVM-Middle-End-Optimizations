; int test_multi_inst(int e, int a) {
;   int b = a + 1;    // -> multi: optmization on d = b - 1
;   int c = b * 3;    // -> multi: optmization on e = c / 3
;                     // -> e = b << 2; c = c - b
;   int d = b - 1;    // -> multi: d = a
;   int f = 3 / c;    // not optimized
;   int e = c / 3;    // -> multi: e = b
;   int g = d + e;    // not optimized
;   return g;
; }

define dso_local i32 @test_multi_inst(i32 noundef %0, i32 noundef %1) #0 {
  %3 = add nsw i32 %1, 1
  %4 = mul nsw i32 %3, 3
  %5 = sub i32 %3, 1
  %6 = udiv i32 3, %4
  %7 = udiv i32 %4, 3
  %8  = add nsw %5, %7
  ret i32 %9
}
