#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define PHONEBOOK_FILE "phonebook.txt"

void error_and_exit(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        write(STDERR_FILENO, "Usage: findPhone <name>\n", 24);
        return 1;
    }

    int pipe1[2], pipe2[2];
    if (pipe(pipe1) == -1) error_and_exit("pipe1");
    if (pipe(pipe2) == -1) error_and_exit("pipe2");

    pid_t pid1 = fork();
    if (pid1 < 0) error_and_exit("fork1");

    if (pid1 == 0) { // Child 1 - grep
        close(pipe1[0]);
        dup2(pipe1[1], STDOUT_FILENO);
        close(pipe1[1]);

        char *grep_args[] = {"grep", NULL, PHONEBOOK_FILE, NULL};
        grep_args[1] = argv[1];
        execvp("grep", grep_args);
        error_and_exit("execvp grep");
    }

    pid_t pid2 = fork();
    if (pid2 < 0) error_and_exit("fork2");

    if (pid2 == 0) { // Child 2 - sed + awk
        close(pipe1[1]);
        dup2(pipe1[0], STDIN_FILENO);
        close(pipe1[0]);

        close(pipe2[0]);
        dup2(pipe2[1], STDOUT_FILENO);
        close(pipe2[1]);

        execlp("sed", "sed", "s/ /#/g", NULL);
        error_and_exit("exec sed");
    }

    pid_t pid3 = fork();
    if (pid3 < 0) error_and_exit("fork3");

    if (pid3 == 0) { // Child 3 - sed ',' to ' '
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[1]);
        dup2(pipe2[0], STDIN_FILENO);
        close(pipe2[0]);

        int pipe3[2];
        if (pipe(pipe3) == -1) error_and_exit("pipe3");
        
        pid_t pid4 = fork();
        if (pid4 < 0) error_and_exit("fork4");

        if (pid4 == 0) { // Child 4 - second sed
            close(pipe3[0]);
            dup2(pipe3[1], STDOUT_FILENO);
            close(pipe3[1]);

            execlp("sed", "sed", "s/,/ /", NULL);
            error_and_exit("exec sed 2");
        }

        close(pipe3[1]);
        dup2(pipe3[0], STDIN_FILENO);
        close(pipe3[0]);

        execlp("awk", "awk", "{print $2}", NULL);
        error_and_exit("exec awk");
    }

    // Parent
    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);

    for (int i = 0; i < 3; i++) {
        wait(NULL);
    }

    return 0;
}
