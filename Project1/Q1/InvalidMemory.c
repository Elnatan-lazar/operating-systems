// InvalidMemory.c
#include <stdio.h>

int main() {
    int *ptr = (int *)0xDEADBEEF; // Invalid memory
    int value = *ptr; // Will make the error
    printf("Value: %d\n", value);
    return 0;
}
