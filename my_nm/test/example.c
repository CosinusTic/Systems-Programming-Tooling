#include <stdio.h>

int do_calc(int a, int b)
{
    return a + b;
}

int main(void)
{
    int a = 24;
    int b = 42;

    printf("%d + %d : ", a, b);
    printf("%d\n ", do_calc(a, b));
    return 0;
}
