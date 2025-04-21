#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <string.h>


void print_help(const char *prog_name) {
    fprintf(stderr, "Usage: %s <real part> <imaginary part> [N]\n", prog_name);
    fprintf(stderr, "Checks if the complex number c = a + bi belongs to the Mandelbrot set.\n");
    fprintf(stderr, "Optional: N is the maximum number of iterations (default: 1000).\n");
}



int main(int argc, char *argv[]) {
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        print_help(argv[0]);
        return 0;
    }

    if (argc < 3) {
        print_help(argv[0]);
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
