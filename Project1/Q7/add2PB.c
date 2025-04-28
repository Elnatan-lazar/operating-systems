#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define PHONEBOOK_FILE "phonebook.txt"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        write(STDERR_FILENO, "Usage: add2PB <name> <phone>\n", 29);
        return 1;
    }

    int fd = open(PHONEBOOK_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // Writing the name
    for (int i = 1; i < argc - 1; i++) {
        write(fd, argv[i], strlen(argv[i]));
        if (i != argc - 2) {
            write(fd, " ", 1);
        }
    }
    write(fd, ",", 1);
    // Writing the phone number
    write(fd, argv[argc-1], strlen(argv[argc-1]));
    write(fd, "\n", 1);
    close(fd);
    
    return 0;
}
