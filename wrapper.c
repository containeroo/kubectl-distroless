#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

volatile sig_atomic_t terminate = 0;

void handle_signal(int sig) {
    terminate = 1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [arguments...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Set up signal handling for SIGINT and SIGTERM
    struct sigaction sa = {0};
    sa.sa_handler = handle_signal;
    sa.sa_flags = SA_RESTART; // Automatically restart interrupted system calls
    if (sigaction(SIGINT, &sa, NULL) == -1 || sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Failed to set signal handlers");
        return EXIT_FAILURE;
    }

    // Fork the process to execute the command
    pid_t pid = fork();
    if (pid < 0) {
        perror("Failed to fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // Child process: Execute the command
        execvp(argv[1], &argv[1]);
        perror("Failed to execute command");
        _exit(EXIT_FAILURE);
    }

    // Parent process: Monitor the child process
    int status;
    while (!terminate) {
        if (waitpid(pid, &status, 0) == -1) {
            if (terminate) break; // Exit loop if signal was caught
            perror("Error while waiting for child process");
            return EXIT_FAILURE;
        }

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status); // Child exited normally
        }

        if (WIFSIGNALED(status)) {
            fprintf(stderr, "Child process terminated by signal %d\n", WTERMSIG(status));
            return EXIT_FAILURE;
        }
    }

    // Handle termination signal
    kill(pid, SIGTERM);
    waitpid(pid, &status, 0); // Ensure child is reaped
    return EXIT_FAILURE;
}

