#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#define _unused __attribute__((unused))

int main(int argc, char **argv, _unused char **envp, _unused char **apple) {
  pid_t pid = getpid();
  char pid_s[6];
  snprintf(pid_s, 6, "%d", pid);
  std::string cwd = getcwd(nullptr, 0);
  int choice = argc > 1 ? atoi(argv[1]) : 2;
  const char *argv0 = "";
  std::string path;

  if (choice > 4) {
    argv0 = choice == 5 ? "sleep" : "getargv";
    choice -= 4;
  }

  switch (choice) {
  case 1:
    return execl("/bin/sleep", argv0, "300", NULL);
  case 2:
    return execl("bin/getargv", argv0, pid_s, NULL);
  case 3:
    return execl("./bin/getargv", argv0, pid_s, NULL);
  case 4:
    path = cwd + "/bin/getargv";
    return execl(path.c_str(), argv0, pid_s, NULL);
  }
}
