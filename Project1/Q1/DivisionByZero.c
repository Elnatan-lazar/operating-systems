#include <stdio.h>

int main()
{
    int a = 1, b = 0;
    printf("%d\n", a / b); // This will cause a division by zero error
    return 0;
}