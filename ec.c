#include <alloca.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "colors.h"

// linked list to store command information
struct ll_cmd {
  char *name;
  char *shell;
  size_t shell_len;
  struct ll_cmd *next;
};

int main(int argc, char *argv[]) {
  // usage check
  if (argc < 3) {
    fprintf(stderr, "usage: %s file command [args...]\n", argv[0]);
    return 1;
  } else if (argc > 102) {
    fprintf(stderr, "error: too many extra arguments (max 99)\n");
    return 1;
  }

  // open the file
  FILE *file = fopen(argv[1], "r");
  if (!file) {
    perror("fopen()");
    return 1;
  }

  // parse the embedded convention header
  struct ll_cmd cmds_head = {0};
  struct ll_cmd *ptr = &cmds_head;
  char *line = NULL;
  size_t linecap = 0;
  ssize_t linelen;
  while ((linelen = getline(&line, &linecap, file)) > 0) {
    // break on an invalid starting line
    if (strncmp(line, "//", 2) && strncmp(line, "##", 2))
      break;

    // get rid of the newline
    line[linelen - 1] = '\0';

    // handle line continuations
    if (line[2] == ' ') {
      // this is a continuation line
      ptr->shell_len += linelen - 2;
      ptr->shell = realloc(ptr->shell, ptr->shell_len);
      strcat(ptr->shell, line + 2);
      continue;
    }

    // if its not a continuation and it doesn't have a separator, then just skip
    char *sep = strchr(line, ':');
    if (!sep)
      continue;

    // make a new linked list node on the stack
    struct ll_cmd *cmd = alloca(sizeof(struct ll_cmd));
    cmd->name = alloca(sep - line - 1);
    strncpy(cmd->name, line + 2, sep - line - 2);
    cmd->shell = strdup(sep + 1); // can't use alloca b/c it might be realloc'd
                                  // by a continuation line
    cmd->shell_len = strlen(sep + 1);
    cmd->next = NULL;

    ptr->next = cmd;
    ptr = ptr->next;
  }
  fclose(file);

  // before we run commands, set the environment variables we need and store the
  // old ones for later
  char *old_envars[argc - 2]; // this array is intentionally one too long
                              // because zero-length variable arrays are UB
  for (int i = 0; i < argc - 3; i++) {
    char num[4];
    snprintf(num, 4, "A%d", i);
    if (!getenv(num)) {
      old_envars[i] = NULL;
    } else {
      old_envars[i] = alloca(strlen(getenv(num)) + 1);
      strcpy(old_envars[i], getenv(num));
    }
    setenv(num, argv[i + 3], 1);
  }
  char *old_src_envar;
  if (!getenv("SRC")) {
    old_src_envar = NULL;
  } else {
    old_src_envar = alloca(strlen(getenv("SRC")) + 1);
    strcpy(old_src_envar, getenv("SRC"));
  }
  setenv("SRC", argv[1], 1);

  // run the commands that we want
  int retval = 0;
  ptr = cmds_head.next;
  int found = 0;
  while (ptr) {
    // if the user asked for this command, then run it
    if (!strcmp(argv[2], ptr->name)) {
      if (!found)
        found = 1;

      printf(SGR_BOLD">> %s"SGR_RESET"\n", ptr->shell);
      // no, this is not bad code. we really do want to use system() here
      int err = system(ptr->shell);
      if (err) {
        retval = 1;
        goto fail;
      }
    }
    free(ptr->shell);
    ptr = ptr->next;
  }

  // if nothing was run, then tell the user
  if (!found) {
    fprintf(stderr, "error: command \"%s\" not found\n", argv[2]);
    retval = 1;
  }

  goto end;
fail:
  // cleanup from a failed system() call
  while (ptr) {
    free(ptr->shell);
    ptr = ptr->next;
  }
end:
  for (int i = 0; i < argc - 3; i++) {
    char num[4];
    snprintf(num, 4, "A%d", i);
    if (!old_envars[i]) {
      unsetenv(num);
      continue;
    }
    setenv(num, old_envars[i], 1);
  }
  if (old_src_envar) {
    setenv("SRC", old_src_envar, 1);
  } else {
    unsetenv("SRC");
  }
  return retval;
}
