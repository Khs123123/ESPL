#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <message>\n", argv[0]);
        exit(1);
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        exit(1);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) {
        // Child: read message from pipe
        close(pipefd[1]); // Close write end
        char buffer[1024];
        int len = read(pipefd[0], buffer, sizeof(buffer) - 1);
        if (len == -1) {
            perror("read failed");
            exit(1);
        }
        buffer[len] = '\0';
        printf("Child received message: %s\n", buffer);
        close(pipefd[0]);
    } else {
        // Parent: write message to pipe
        close(pipefd[0]); // Close read end
        if (write(pipefd[1], argv[1], strlen(argv[1])) == -1) {
            perror("write failed");
            exit(1);
        }
        close(pipefd[1]);
        wait(NULL); // Wait for child to finish
    }

    return 0;
}