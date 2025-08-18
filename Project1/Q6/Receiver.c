#include <stdio.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t bits[8];
volatile sig_atomic_t bit_index = 0;
volatile sig_atomic_t received = 0;

void handle_sigusr1(int sig) {
    (void)sig;
    bits[bit_index++] = 0;
    if (bit_index == 8) received = 1;
}

void handle_sigusr2(int sig) {
    (void)sig;
    bits[bit_index++] = 1;
    if (bit_index == 8) received = 1;
}

int main() {
    printf("My PID is %d\n", getpid());

    struct sigaction sa1 = {0}, sa2 = {0};
    sigset_t full_mask;
    sigemptyset(&full_mask);
    sigaddset(&full_mask, SIGUSR1);
    sigaddset(&full_mask, SIGUSR2);

    sa1.sa_handler = handle_sigusr1;
    sa1.sa_mask = full_mask;
    sa1.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sa1, NULL);

    sa2.sa_handler = handle_sigusr2;
    sa2.sa_mask = full_mask;
    sa2.sa_flags = SA_RESTART;
    sigaction(SIGUSR2, &sa2, NULL);

    while (1) {
        pause();    
        if (received) {
            int number = 0;
            for (int i = 0; i < 8; ++i) {
                number = (number << 1) | bits[i];
            }
            printf("Received %d\n", number);
            bit_index = 0;
            received = 0;
        }
    }
    
    return 0;
}