#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dispatch/dispatch.h>
#include <unistd.h>

void exitOnParentExit(void) {
  dispatch_source_t source = dispatch_source_create(
                                                    DISPATCH_SOURCE_TYPE_PROC, getppid(), DISPATCH_PROC_EXIT, NULL);

  dispatch_source_set_event_handler(source, ^{
    (void)source; // capture so lifetime is long enough
    exit(0);
  });

  dispatch_resume(source);
}

int main() {
  pid_t pid = getppid();

  exitOnParentExit();
  // signal to parent that child is ready
  if (0 != kill(pid, SIGCONT)) {
    perror("child");
    exit(EXIT_FAILURE);
  }

  // wait for parent to be done
  pause();
}
