#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

int main() {
  pid_t pid = getppid();

  // signal to parent that child is ready
  if (0 != kill(pid, SIGCONT)) {
    perror("child");
    exit(EXIT_FAILURE);
  }

  // wait for parent to be done
  pause();
}
