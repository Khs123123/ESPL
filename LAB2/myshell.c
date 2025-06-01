#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include "LineParser.h"
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <linux/limits.h>

void execute(cmdLine *pCmdLine, int debug);
void changeDir(cmdLine *parseCmd, int debug);
void handleSignalCmd(cmdLine *parseCmd, char *proc, int debug);
void redirection(cmdLine *parseCmd, int debug);

int main(int argc, char **argv) {
    int debug = 0;
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "-d", 2) == 0) {
            debug = 1;
        }
    }

    while (1) {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s> ", cwd);
        } else {
            perror("getcwd failed");
            exit(1);
        }

        char input[2048];
        if (fgets(input, sizeof(input), stdin) == NULL) {
            if (debug) perror("Error reading the line!");
            continue;
        }

        if (strncmp(input, "quit", 4) == 0) exit(0);

        cmdLine *cmd = parseCmdLines(input);
        if (strcmp(cmd->arguments[0], "cd") == 0) {
            changeDir(cmd, debug);
        } else if (
            strcmp(cmd->arguments[0], "halt") == 0 ||
            strcmp(cmd->arguments[0], "wakeup") == 0 ||
            strcmp(cmd->arguments[0], "ice") == 0
        ) {
            handleSignalCmd(cmd, cmd->arguments[0], debug);
        } else {
            execute(cmd, debug);
        }

        freeCmdLines(cmd);
    }

    return 0;
}

void changeDir(cmdLine *cmd, int debug) {
    if (chdir(cmd->arguments[1]) == -1) {
        if (debug) perror("cd failed");
    }
}

void redirection(cmdLine *cmd, int debug) {
    if (cmd->inputRedirect) {
        if (!freopen(cmd->inputRedirect, "r", stdin) && debug) {
            perror("Input redirection failed");
        }
    }
    if (cmd->outputRedirect) {
        if (!freopen(cmd->outputRedirect, "w", stdout) && debug) {
            perror("Output redirection failed");
        }
    }
}

void execute(cmdLine *cmd, int debug) {
    pid_t pid = fork();
    if (pid == -1) {
        if (debug) perror("fork failed");
        exit(1);
    }

    if (pid == 0) {
        redirection(cmd, debug);
        if (execvp(cmd->arguments[0], cmd->arguments) == -1) {
            if (debug) perror("execvp failed");
            _exit(1);
        }
    } else {
        if (debug) {
            fprintf(stderr, "pid = %d\n", pid);
            fprintf(stderr, "Executing command: %s\n", cmd->arguments[0]);
        }

        if (cmd->blocking) {
            if (waitpid(pid, NULL, 0) == -1 && debug) {
                perror("waitpid failed");
            }
        }
    }
}

void handleSignalCmd(cmdLine *cmd, char *proc, int debug) {
    pid_t pid = atoi(cmd->arguments[1]);

    int sig = 0;
    if (strcmp(proc, "wakeup") == 0) sig = SIGCONT;
    else if (strcmp(proc, "ice") == 0) sig = SIGINT;
    else if (strcmp(proc, "halt") == 0) sig = SIGTSTP;

    if (kill(pid, sig) == -1 && debug) {
        fprintf(stderr, "Failed to send signal '%s' to pid %d\n", proc, pid);
        perror("kill");
    }
}