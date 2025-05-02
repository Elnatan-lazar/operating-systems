#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define PHONEBOOK_FILE "phonebook.txt"

int main(int argc, char *argv[]) {
    if (argc < 4) {
        write(STDERR_FILENO, "Usage: add2PB <name...> , <phone>\n", 35);
        return 1;
    }

    int fd = open(PHONEBOOK_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // Find the index of the comma
    int comma_index = -1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], ",") == 0) {
            comma_index = i;
            break;
        }
    }

    // Error if no comma or comma is last argument
    if (comma_index == -1 || comma_index == argc - 1) {
        write(STDERR_FILENO, "Error: missing comma or phone number.\n", 39);
        close(fd);
        return 1;
    }

    // Write the name with spaces
    for (int i = 1; i < comma_index; i++) {
        write(fd, argv[i], strlen(argv[i]));
        if (i != comma_index - 1) {
            write(fd, " ", 1);
        }
    }

    // Write comma and phone number
    write(fd, ",", 1);
    write(fd, argv[comma_index + 1], strlen(argv[comma_index + 1]));
    write(fd, "\n", 1);

    close(fd);
    return 0;
}
