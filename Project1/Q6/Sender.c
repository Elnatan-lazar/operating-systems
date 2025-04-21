#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void send_bit(pid_t pid, int bit) {
    int sig = (bit==0) ? SIGUSR1 : SIGUSR2; // If 0 -> SIGUSR1, else -> SIPUSR2
    kill(pid, sig);
    usleep(100000); // 0.1 second
}

int main() {
    pid_t receiver_pid;
    int number;

    printf("Enter receiver PID: ");
    scanf("%d", &receiver_pid);

    printf("Enter message: ");
    scanf("%d", &number);

    if (number<0 || number>255) {
        fprintf(stderr, "Invalid number.\n");
        return 1;
    }

    // Send the bits from the MSB
    for (int i=7 ; i>=0 ; i--) {
        int bit = (number >> i) & 1;
        send_bit(receiver_pid, bit);
    }

    return 0;
}