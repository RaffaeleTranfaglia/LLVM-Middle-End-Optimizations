; int test_multi_inst(int e, int a) {
;   int b = a + 1;    // -> multi: optimization on d = b - 1
;   int c = b * 3;    // -> multi: optimization on g = c / 3
;                     // -> c = b << 2; c = c - b
;   int d = b - 1;    // -> multi: d = a; -> deleted
;   int f = 3 / c;    // not optimizated
;   int g = c / 3;    // -> multi: g = b; -> deleted
;   int h = c + d;    // -> h = c + a
;   int i = f + g;    // -> i = f + a 
;   int j = h * i     // not optimized
;   return j;
; }

define dso_local i32 @test_multi_inst(i32 noundef %0, i32 noundef %1) #0 {
  %3 = add nsw i32 %1, 1
  %4 = mul nsw i32 %3, 3
  %5 = sub i32 %3, 1
  %6 = udiv i32 3, %4
  %7 = udiv i32 %4, 3
  %8 = add nsw i32 %4, %5
  %9 = add nsw i32 %6, %7
  %10 = mul nsw i32 %8, %9
  ret i32 %10
}
