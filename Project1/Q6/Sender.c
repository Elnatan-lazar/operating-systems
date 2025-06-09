#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void send_bit(pid_t pid, int bit) {
    int sig = (bit == 0) ? SIGUSR1 : SIGUSR2;

    if (kill(pid, sig) == -1) {
        perror("Failed to send signal");
        exit(1);
    }

    usleep(100000); // 0.1 second delay to ensure signal is processed
}

int main() {
    pid_t receiver_pid;
    int number;

    printf("Enter receiver PID: ");
    if (scanf("%d", &receiver_pid) != 1) {
        fprintf(stderr, "Failed to read PID.\n");
        return 1;
    }

    printf("Enter message (0-255): ");
    if (scanf("%d", &number) != 1 || number < 0 || number > 255) {
        fprintf(stderr, "Invalid number. Must be between 0 and 255.\n");
        return 1;
    }

    // Send the bits from MSB to LSB
    for (int i = 7; i >= 0; i--) {
        int bit = (number >> i) & 1;
        send_bit(receiver_pid, bit);
    }

    printf("Sent %d to process %d successfully.\n", number, receiver_pid);
    return 0;
}
