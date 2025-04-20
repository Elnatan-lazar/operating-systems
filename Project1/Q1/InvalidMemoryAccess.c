// InvalidMemory.c
#include <stdio.h>

int main() {
    int *ptr = (int *)0xDEADBEEF; // כתובת לא חוקית בזיכרון
    int value = *ptr; // ניסיון קריאה – תגרום לשגיאה/קריסה
    printf("Value: %d\n", value);
    return 0;
}
//
// Created by elnatan on 20/04/2025.
//
