#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char const *argv[]) {
    int pipe_fd[2];
    pid_t child1, child2;

    // creating pipe
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // forking the first child
    fprintf(stderr, "(parent_process>forking...)\n");

    child1 = fork();
    if (child1 == -1) {
        perror("fork error (child1)");
        exit(EXIT_FAILURE);
    }

    if (child1 == 0) {
        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe...)\n");

        close(STDOUT_FILENO);             // Close stdout
        dup(pipe_fd[1]);                  // Duplicate write end of the pipe to stdout
        close(pipe_fd[1]);                // Close the original write end
        close(pipe_fd[0]);                // Close the read end (not needed in this child)

        fprintf(stderr, "(child1>going to execute cmd: ls -ls)\n");
        char *args[] = {"ls", "-ls", NULL};
        execvp(args[0], args);            // Execute ls -ls
        perror("error in executing ls -ls");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "(parent_process>created process with id: %d)\n", child1);
    fprintf(stderr, "(parent_process>closing the write end of the pipe...)\n");
    close(pipe_fd[1]);                    // Close write end in parent

    fprintf(stderr, "(parent_process>forking...)\n");
    child2 = fork();
    if (child2 == -1) {
        perror("fork error (child2)");
        exit(EXIT_FAILURE);
    }

    if (child2 == 0) {
        fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe...)\n");

        close(STDIN_FILENO);              // Close stdin
        dup(pipe_fd[0]);                  // Duplicate read end of the pipe to stdin
        close(pipe_fd[0]);                // Close the original read end

        fprintf(stderr, "(child2>going to execute cmd: wc)\n");
        char *args2[] = {"wc", NULL};
        execvp(args2[0], args2);          // Execute wc
        perror("error while executing wc");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "(parent_process>created process with id: %d)\n", child2);
    fprintf(stderr, "(parent_process>closing the read end of the pipe...)\n");
    close(pipe_fd[0]);                    // Close read end in parent

    fprintf(stderr, "(parent_process>waiting for child processes to terminate...)\n");

    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);

    fprintf(stderr, "(parent_process>exiting...)\n");

    return 0;
}