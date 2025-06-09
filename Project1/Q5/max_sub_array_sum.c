#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MIN_NUM -25
#define MAX_NUM 74

// Creating random array
void generate_random_array(int *arr, int size, unsigned int seed) {
    srand(seed);
    for (int i = 0; i < size; i++) {
        arr[i] = (rand() % (MAX_NUM - MIN_NUM + 1)) + MIN_NUM;
    }
}

// O(n³)
int max_subarray_sum_n3(int *arr, int size) {
    int max_sum = arr[0];
    for (int i = 0; i < size; i++) {
        for (int j = i; j < size; j++) {
            int sum = 0;
            for (int k = i; k <= j; k++) {
                sum += arr[k];
            }
            if (sum > max_sum) {
                max_sum = sum;
            }
        }
    }
    return max_sum;
}

// O(n²)
int max_subarray_sum_n2(int *arr, int size) {
    int max_sum = arr[0];
    for (int i = 0; i < size; i++) {
        int sum = 0;
        for (int j = i; j < size; j++) {
            sum += arr[j];
            if (sum > max_sum) {
                max_sum = sum;
            }
        }
    }
    return max_sum;
}

// O(n)
int max_subarray_sum_n(int *arr, int size) {
    int max_current = arr[0];
    int max_global = arr[0];
    for (int i = 1; i < size; i++) {
        if (arr[i] > max_current + arr[i]) {
            max_current = arr[i];
        } else {
            max_current += arr[i];
        }
        if (max_current > max_global) {
            max_global = max_current;
        }
    }
    return max_global;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <seed> <size>\n", argv[0]);
        return 1;
    }

    unsigned int seed = (unsigned int)atoi(argv[1]);
    int size = atoi(argv[2]);
    if (size <= 0) {
        printf("Invalid array size.\n");
        return 1;
    }

    int *arr = (int *)malloc(size * sizeof(int));
    if (!arr) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    clock_t start_generate = clock();
    generate_random_array(arr, size, seed);
    clock_t end_generate = clock();
    printf("Generated %d random numbers.\n", size);

    clock_t start_alg, end_alg;
    int result;

    // O(n³)
    start_alg = clock();
    result = max_subarray_sum_n3(arr, size);
    end_alg = clock();
    printf("O(n^3) result = %d | Time = %lf seconds\n", result, (double)(end_alg - start_alg) / CLOCKS_PER_SEC);

    // O(n²)
    start_alg = clock();
    result = max_subarray_sum_n2(arr, size);
    end_alg = clock();
    printf("O(n^2) result = %d | Time = %lf seconds\n", result, (double)(end_alg - start_alg) / CLOCKS_PER_SEC);

    // O(n)
    start_alg = clock();
    result = max_subarray_sum_n(arr, size);
    end_alg = clock();
    printf("O(n) result = %d | Time = %lf seconds\n", result, (double)(end_alg - start_alg) / CLOCKS_PER_SEC);
    
    printf("Random generation time = %lf seconds\n", (double)(end_generate - start_generate) / CLOCKS_PER_SEC);
    free(arr);

    return 0;
}
