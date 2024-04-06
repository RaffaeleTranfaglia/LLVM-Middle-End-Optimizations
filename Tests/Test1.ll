; int test_algebraic_identity(int e, int a) {
;   int b = a + 0;  // -> b = a; -> deleted
;   int c = 0 - b;  // -> c = 0 - a
;   int d = e / 1;  // -> d = e; -> deleted
;   int f = e - 0;  // -> f = e; -> deleted
;   int g = b + c;  // -> g = a + c
;   int h = d + f;  // -> h = e + e
;   return g * h;
; }

define dso_local i32 @test_algebraic_identity(i32 noundef %0, i32 noundef %1) #0 {
  %3 = add nsw i32 %1, 0
  %4 = sub i32 0, %3
  %5 = udiv i32 %0, 1
  %6 = sub i32 %0, 0
  %7 = add i32 %3, %4
  %8 = add i32 %5, %6
  %9 = mul nsw i32 %7, %8
  ret i32 %9
}
