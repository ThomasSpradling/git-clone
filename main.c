#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "pit.h"

int is_initialized() {
  // ensures that directory `.pit` exists and is a directory
  struct stat s;
  int ret_code = stat(".pit", &s);
  return ret_code != -1 && S_ISDIR(s.st_mode);
}

int is_valid_filename(const char *filename) {
  if (strlen(filename) > FILENAME_SIZE - 1 || strlen(filename) == 0 || filename[0] == '.') {
    return 0;
  }

  // ensures that the file exists and is not a directory
  struct stat s;
  int ret_code = stat(filename, &s);
  return ret_code != -1 && !(S_ISDIR(s.st_mode));
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <command> [<args>]\n", argv[0]);
    return 2;
  }

  // pit init
  if (strcmp(argv[1], "init") == 0) {
    if (is_initialized()) {
      fprintf(stderr, "ERROR: Repo is already initialized!\n");
      return 1;
    }

    return pit_init();
  }

  if (!is_initialized()) {
    fprintf(stderr, "ERROR: Repo is not initialized yet!\n");
    return 1;
  }

  // pit add <filename>
  if (strcmp(argv[1], "add") == 0) {
    if (argc < 3 || !is_valid_filename(argv[2])) {
      fprintf(stderr, "ERROR: No or invalid filname given.\n");
      return 1;
    }

    return pit_add(argv[2]);
  }

  // pit rm <filename>
  if (strcmp(argv[1], "rm") == 0) {
    if (argc < 3 || !is_valid_filename(argv[2])) {
      fprintf(stderr, "ERROR: No or invalid filname given.\n");
      return 1;
    }

    return pit_rm(argv[2]);
  }

  // pit commit -m <message>
  if (strcmp(argv[1], "commit") == 0) {
    if (argc < 4 || strcmp(argv[2], "-m") != 0) {
      fprintf(stderr, "ERROR: Must have a commit message (-m <message>).\n");
      return 1;
    }

    if (strlen(argv[3]) > MESSAGE_SIZE - 1) {
      fprintf(stderr, "ERROR: Message is too long!\n");
      return 1;
    }

    return pit_commit(argv[3]);
  }

  // pit status
  if (strcmp(argv[1], "status") == 0) {
    return pit_status();
  }

  // pit log
  if (strcmp(argv[1], "log") == 0) {
    return pit_log();
  }

  fprintf(stderr, "ERROR: Unknown command \"%s\"!\n", argv[1]);
  return 1;
}