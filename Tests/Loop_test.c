// Before optmizing with loopopts, optmimize the .ll with opt mem2reg
#include <stdio.h>

void foo(int c, int z) {
  int a = 9, h, m = 0, n = 0, q, r = 0, y = 0;

LOOP:
  z = z + 1;  // not loop invariant
  y = c + 3;  // code motion candidate = loop invariant, since c is defined before the loop; dominates its uses;
              // dominates the only exit
  q = c + 7;  // code motion candidate = loop invariant; dominates its uses; dominates the only exit
  if (z < 5) {
    a = a + 2;  // not loop invariant
    h = c + 3;  // does not dominates all uses, not the exit
  } else {
    a = a - 1;  // not loop invariant
    h = c + 4;   // does not dominates all uses,
    if (z >= 10) {
      goto EXIT;
    }
  }
  m = y + 7;  // doesn't dominate the exits and it's not dead
  n = h + 2;  // doesn't dominate the exits and it's not dead
  y = c + 7;  // code motion candidate: loop invariant; dominates its uses, since it has none; dead,
              // since the the printf prints the previous y, which dominates the exit
  r = q + 5;  // doesn't dominate the exits and it's not dead
  goto LOOP;
EXIT:
  printf("%d,%d,%d,%d,%d,%d,%d,%d\n", a, h, m, n, q, r, y, z);
}

int main() {
  foo(0, 4);
  foo(0, 12);
  return 0;
}
