#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

// Calls the finalize script.
// This function shall be called only by the main process
void call_finalize_script(char *const script_path) {
  pid_t child_pid = fork();
  if (child_pid == 0) { // in child
    printf("Ready to call final script %s\n", script_path);
    char *const args[] = {script_path, NULL};
    execvp(args[0], args);
    printf("If we got here, we had some kind of error...\n");
  } else if (child_pid < 0) {
    printf("Could not inform the world of my completion\n");
    printf("Will just terminate silently...\n");
  }
}