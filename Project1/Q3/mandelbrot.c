#include <complex.h>
#include <math.h>
#include <stdbool.h>

bool is_in_mandelbrot(complex double c, int N) {
    complex double z = 0.0 + 0.0 * I;
    double M = 2.0;

    for (int i = 0; i < N; ++i) {
        z = z * z + c;
        if (cabs(z) > M) {
            return false;
        }
    }
    return true;
}
