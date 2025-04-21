#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <real part> <imaginary part> [N]\n", argv[0]);
        return 1;
    }

    double real = atof(argv[1]);
    double imag = atof(argv[2]);
    int N = (argc > 3) ? atoi(argv[3]) : 1000;
    double M = 2.0;

    double complex c = real + imag * I;
    double complex z = 0.0 + 0.0 * I;

    for (int i = 0; i < N; i++) {
        z = z*z + c;

        if (cabs(z) > M) {
            printf("The sequence diverges. The number is NOT in the Mandelbrot set.\n");
            return 0;
        }
    }

    printf("The sequence does NOT diverge. The number is in the Mandelbrot set.\n");
    return 0;
}
