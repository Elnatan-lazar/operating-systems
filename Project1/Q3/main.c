#include <stdio.h>
#include <complex.h>
#include "mandelbrot.h"

int main() {
    double real, imag;
    int N = 1000;

    while (1) {
        printf("Enter real and imaginary parts: ");
        scanf("%lf %lf", &real, &imag);

        if (real == 0 && imag == 0) {
            printf("Stopping: 0 + 0i is always in the Mandelbrot set.\n");
            break;
        }

        complex double c = real + imag * I;
        if (is_in_mandelbrot(c, N)) {
            printf("The number is in the Mandelbrot set.\n");
        } else {
            printf("The number is NOT in the Mandelbrot set.\n");
        }
    }

    return 0;
}
//
// Created by elnatan on 21/04/2025.
//
