#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include "LineParser.h"
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>

#define HISTLEN 20
#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0

typedef struct process {
    char *commandName;
    pid_t pid;
    int status;
    struct process *next;
} process;

process *process_list = NULL;

typedef struct historyNode {
    char *command;
    struct historyNode *next;
} historyNode;

historyNode *history_head = NULL;
historyNode *history_tail = NULL;
int history_size = 0;

void redirection(cmdLine *cmd, int debug);
void addProcess(process **process_list, cmdLine *cmd, pid_t pid);
void updateProcessList(process **process_list);
void updateProcessStatus(process *process_list, int pid, int status);
void printProcessList(process **process_list);
void execute(cmdLine *cmd, int debug);
void execute_pipeline(cmdLine *cmd, int debug);
void addHistory(const char *cmd);
void printHistory();
void executeLastHistory(int debug);
void executeNthHistory(int n, int debug);
void freeHistory();
void changeDir(cmdLine *cmd, int debug);

int isInternalCommand(const char *cmd) {
    return (strcmp(cmd, "cd") == 0 || strcmp(cmd, "procs") == 0 ||
            strcmp(cmd, "wakeup") == 0 || strcmp(cmd, "halt") == 0 ||
            strcmp(cmd, "ice") == 0 || strcmp(cmd, "hist") == 0 ||
            strcmp(cmd, "!!") == 0 || cmd[0] == '!');
}

int main(int argc, char **argv) {
    int debug = 0;
    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "-d") == 0) debug = 1;

    while (1) {
        char cwd[PATH_MAX];
        getcwd(cwd, PATH_MAX);
        printf("%s> ", cwd);

        char input[2048];
        if (!fgets(input, sizeof(input), stdin)) continue;
        if (strncmp(input, "quit", 4) == 0) break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "hist") == 0) {
            printHistory();
            continue;
        } else if (strcmp(input, "!!") == 0) {
            executeLastHistory(debug);
            continue;
        } else if (input[0] == '!' && isdigit(input[1])) {
            int n = atoi(&input[1]);
            executeNthHistory(n, debug);
            continue;
        }

        addHistory(input);
        cmdLine *cmd = parseCmdLines(input);

        if (strcmp(cmd->arguments[0], "cd") == 0) {
            changeDir(cmd, debug);
        } else if (strcmp(cmd->arguments[0], "procs") == 0) {
            printProcessList(&process_list);
        } else if (strcmp(cmd->arguments[0], "wakeup") == 0 ||
                   strcmp(cmd->arguments[0], "halt") == 0 ||
                   strcmp(cmd->arguments[0], "ice") == 0) {
            pid_t pid = atoi(cmd->arguments[1]);
            int sig = 0, status = -999;
            if (strcmp(cmd->arguments[0], "wakeup") == 0) {
                sig = SIGCONT; status = RUNNING;
            } else if (strcmp(cmd->arguments[0], "halt") == 0) {
                sig = SIGTSTP; status = SUSPENDED;
            } else if (strcmp(cmd->arguments[0], "ice") == 0) {
                sig = SIGINT; status = TERMINATED;
            }
            if (kill(pid, sig) == -1) perror("kill failed");
            else updateProcessStatus(process_list, pid, status);
        } else if (cmd->next != NULL) {
            execute_pipeline(cmd, debug);
        } else if (!isInternalCommand(cmd->arguments[0])) {
            execute(cmd, debug);
        }

        freeCmdLines(cmd);
    }

    freeHistory();
    return 0;
}

// ----------------- Execution Functions -----------------

void execute(cmdLine *cmd, int debug) {
    // Check if the command is internal before forking
    if (strcmp(cmd->arguments[0], "procs") == 0 ||
        strcmp(cmd->arguments[0], "halt") == 0 ||
        strcmp(cmd->arguments[0], "wakeup") == 0 ||
        strcmp(cmd->arguments[0], "ice") == 0 ||
        strcmp(cmd->arguments[0], "cd") == 0 ||
        strcmp(cmd->arguments[0], "hist") == 0 ||
        strcmp(cmd->arguments[0], "!!") == 0 ||
        cmd->arguments[0][0] == '!') {
        // Do NOT fork or add these commands to the process list!
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {  // CHILD
        redirection(cmd, debug);
        execvp(cmd->arguments[0], cmd->arguments);
        perror("exec failed");
        _exit(1);
    } else if (pid > 0) {  // PARENT
        addProcess(&process_list, cmd, pid);
        if (debug) {
            fprintf(stderr, "pid = %d\n", pid);
            fprintf(stderr, "Executing command = %s\n", cmd->arguments[0]);
        }
        if (cmd->blocking) waitpid(pid, NULL, 0);
    } else {
        perror("fork failed");
    }
}

void execute_pipeline(cmdLine *cmd, int debug) {
    int pipe_fd[2];
    pipe(pipe_fd);

    pid_t pid1 = fork();
    if (pid1 == 0) {
        close(pipe_fd[0]);                       // Close read end
        dup2(pipe_fd[1], STDOUT_FILENO);         // Redirect stdout to write end of pipe
        close(pipe_fd[1]);                       // Close original write end after dup
        redirection(cmd, debug);                 // Apply redirection if needed
        execvp(cmd->arguments[0], cmd->arguments);
        perror("exec failed (left side)");
        _exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        close(pipe_fd[1]);                       // Close write end
        dup2(pipe_fd[0], STDIN_FILENO);          // Redirect stdin to read end of pipe
        close(pipe_fd[0]);                       // Close original read end after dup
        redirection(cmd->next, debug);           // Apply redirection if needed
        execvp(cmd->next->arguments[0], cmd->next->arguments);
        perror("exec failed (right side)");
        _exit(1);
    }

    // Parent process closes both ends of the pipe
    close(pipe_fd[0]);
    close(pipe_fd[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

// ----------------- Process Manager -----------------

void addProcess(process **process_list, cmdLine *cmd, pid_t pid) {
    // Skip internal commands
    if (strcmp(cmd->arguments[0], "procs") == 0 ||
        strcmp(cmd->arguments[0], "halt") == 0 ||
        strcmp(cmd->arguments[0], "wakeup") == 0 ||
        strcmp(cmd->arguments[0], "ice") == 0 ||
        strcmp(cmd->arguments[0], "cd") == 0 ||
        strcmp(cmd->arguments[0], "hist") == 0 ||
        strcmp(cmd->arguments[0], "!!") == 0 ||
        cmd->arguments[0][0] == '!') {
        return;
    }

    process *new_process = malloc(sizeof(process));
    new_process->commandName = strdup(cmd->arguments[0]); // Save only the name
    new_process->pid = pid;
    new_process->status = RUNNING;
    new_process->next = *process_list;
    *process_list = new_process;
}

void updateProcessStatus(process *list, int pid, int status) {
    while (list) {
        if (list->pid == pid) {
            list->status = status;
            break;
        }
        list = list->next;
    }
}

void updateProcessList(process **process_list) {
    process *p = *process_list;
    int status;
    while (p) {
        pid_t result = waitpid(p->pid, &status, WNOHANG | WUNTRACED | WCONTINUED);
        if (result == -1 || (result > 0 && WIFEXITED(status))) p->status = TERMINATED;
        else if (result > 0 && WIFSTOPPED(status)) p->status = SUSPENDED;
        else if (result > 0 && WIFCONTINUED(status)) p->status = RUNNING;
        p = p->next;
    }
}

void printProcessList(process **process_list) {
    updateProcessList(process_list);
    process *curr = *process_list;
    process *prev = NULL;
    int idx = 0;

    printf("INDEX\tPID\tSTATUS\t\tCOMMAND\n");
    while (curr) {
        printf("%d\t%d\t", idx++, curr->pid);
        if (curr->status == TERMINATED) printf("TERMINATED\t");
        else if (curr->status == RUNNING) printf("RUNNING\t\t");
        else if (curr->status == SUSPENDED) printf("SUSPENDED\t");

        printf("%s\n", curr->commandName);

        if (curr->status == TERMINATED) {
            process *to_delete = curr;
            if (prev == NULL) *process_list = curr->next;
            else prev->next = curr->next;
            curr = curr->next;
            free(to_delete->commandName);
            free(to_delete);
        } else {
            prev = curr;
            curr = curr->next;
        }
    }
}

// ----------------- History -----------------

void addHistory(const char *cmd) {
    if (history_size == HISTLEN) {
        historyNode *temp = history_head;
        history_head = history_head->next;
        free(temp->command);
        free(temp);
        history_size--;
    }
    historyNode *new_node = malloc(sizeof(historyNode));
    new_node->command = strdup(cmd);
    new_node->next = NULL;
    if (!history_tail) {
        history_head = history_tail = new_node;
    } else {
        history_tail->next = new_node;
        history_tail = new_node;
    }
    history_size++;
}

void printHistory() {
    historyNode *cur = history_head;
    int i = 1;
    while (cur) {
        printf("%d: %s\n", i++, cur->command);
        cur = cur->next;
    }
}

void executeLastHistory(int debug) {
    if (!history_tail) {
        printf("No history available\n");
        return;
    }
    cmdLine *cmd = parseCmdLines(history_tail->command);
    if (cmd->next) execute_pipeline(cmd, debug);
    else execute(cmd, debug);
    freeCmdLines(cmd);
}

void executeNthHistory(int n, int debug) {
    if (n < 1 || n > history_size) {
        printf("Invalid history index\n");
        return;
    }
    historyNode *cur = history_head;
    for (int i = 1; i < n; i++) cur = cur->next;
    cmdLine *cmd = parseCmdLines(cur->command);
    if (cmd->next) execute_pipeline(cmd, debug);
    else execute(cmd, debug);
    freeCmdLines(cmd);
}

void freeHistory() {
    historyNode *cur = history_head;
    while (cur) {
        historyNode *tmp = cur;
        cur = cur->next;
        free(tmp->command);
        free(tmp);
    }
    history_head = history_tail = NULL;
    history_size = 0;
}

// ----------------- Misc -----------------

void redirection(cmdLine *cmd, int debug) {
    if (cmd->inputRedirect)
        freopen(cmd->inputRedirect, "r", stdin);
    if (cmd->outputRedirect)
        freopen(cmd->outputRedirect, "w", stdout);
}

void changeDir(cmdLine *cmd, int debug) {
    if (chdir(cmd->arguments[1]) == -1 && debug)
        perror("cd failed");
}