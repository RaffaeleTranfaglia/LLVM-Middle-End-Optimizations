; int test_const_fold(int a) {
;   int b = 2 + 5;
;   int c = 5 * 8
;   int d = 10 - 7
;   int e = 8 / 4
;   int f = b + c
;   int g = d + e
;   return f * g;
; }

define dso_local i32 @test_const_fold(i32 noundef %0) #0 {
  %2 = add nsw i32 2, 5
  %3 = mul nsw i32 5, 8
  %4 = sub i32 10, 7
  %5 = udiv i32 8, 4
  %6 = add nsw i32 %2, %3
  %7 = add nsw i32 %4, %5
  %8 = mul nsw i32 %6, %7
  ret i32 %8
}