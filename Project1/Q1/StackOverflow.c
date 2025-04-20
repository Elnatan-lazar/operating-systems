// StackOverflow.c
#include <stdio.h>

void recurse() {
    int arr[1000]; // צור עומס מלאכותי על המחסנית
    printf("Recursing...\n");
    recurse(); // קריאה רקורסיבית אינסופית
}

int main() {
    recurse();
    return 0;
}
