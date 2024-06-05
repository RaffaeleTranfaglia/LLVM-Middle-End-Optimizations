#include <stdio.h>

void foo(int N, int *b, int *c) {
    int a[N], d[N];

    for (int i=0; i<N; i++)
        a[i+1] = 1/b[i]*c[i];
    for (int i=0; i<N; i++)
        d[i] = a[i]+c[i];
}
