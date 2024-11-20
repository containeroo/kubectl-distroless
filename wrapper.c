#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

volatile sig_atomic_t terminate = 0;

void handle_signal(int sig) { terminate = 1; }

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <command> [arguments...]\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Set up signal handling for SIGINT and SIGTERM
  struct sigaction sa = {0};
  sa.sa_handler = handle_signal;
  sa.sa_flags = SA_RESTART; // Restart system calls like waitpid
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);

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
    // Wait for the child process (blocking wait)
    pid_t result = waitpid(pid, &status, 0);
    if (result == pid) {
      // Child process has terminated
      if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
      }
      if (WIFSIGNALED(status)) {
        fprintf(stderr, "Child process terminated by signal %d\n",
                WTERMSIG(status));
        return EXIT_FAILURE;
      }
    }
    // Check if a signal interrupted the wait
    if (result < 0) {
      if (terminate) {
        // If terminated, send SIGTERM to child and exit
        kill(pid, SIGTERM);
        waitpid(pid, &status, 0); // Ensure child is reaped
        return EXIT_FAILURE;
      }
      perror("Error while waiting for child process");
      return EXIT_FAILURE;
    }
  }

  // If a signal interrupted the wait
  kill(pid, SIGTERM);
  waitpid(pid, &status, 0); // Ensure child is reaped
  return EXIT_FAILURE;
}
