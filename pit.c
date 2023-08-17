#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "pit.h"
#include "utils.h"

/* Usage: `pit init`
 *
 * Creates a `.pit` directory with created `.index`, `.prev`, `.current_branch`, and `.branches` files
 */
int pit_init() {
  fs_mkdir(".pit");
  fs_write_file(".pit/.index", NULL);
  fs_write_file(".pit/.prev", BASE_COMMIT_ID);

  fs_write_file(".pit/.branches", "master\n");
  fs_write_file(".pit/.current_branch", "master");
  return 0;
}

/* Usage: `pit add <filename>`
 *
 * Appends filename to list in `.index`
 * 
 * Errors:
 * >> ERROR: File <filename> already added
 * 
 */
int pit_add(const char *filename) {
  // Creates a newindex to later copy into for atomicity purposes
  FILE *findex = fs_open(".pit/.index", "r");
  FILE *fnewindex = fs_open(".pit/.newindex", "w");

  // Goes line-by-line through .index
  char line[FILENAME_SIZE + 1];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n"); // remove \n from the line

    // if we've seen this file before, stop process and throw error
    if (strcmp(line, filename) == 0) {
      fprintf(stderr, "ERROR: File %s is already tracked!\n", filename);
      fclose(findex);
      fclose(fnewindex);
      fs_rm(".pit/.newindex");
      return 3;
    }

    fprintf(fnewindex, "%s\n", line);
  }
  fprintf(fnewindex, "%s\n", filename);

  fclose(findex);
  fclose(fnewindex);
  fs_mv(".pit/.newindex", ".pit/.index");

  return 0;
}

/* Usage: `pit rm <filename>`
 *
 * Removes the specified filename from `.index`
 * 
 * Errors:
 * >> ERROR: ERROR: File <filename> already not tracked and cannot be removed.
 */
int pit_rm(const char *filename) {
  FILE *findex = fs_open(".pit/.index", "r");
  FILE *fnewindex = fs_open(".pit/.newindex", "w");

  int has_found_value = 0;

  char line[FILENAME_SIZE + 1];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");

    if (strcmp(filename, line) != 0) {
      fprintf(fnewindex, "%s\n", line);
    } else {
      has_found_value = 1;
    }
  }

  fclose(findex);
  fclose(fnewindex);
  fs_mv(".pit/.newindex", ".pit/.index");

  if (!has_found_value) {
    fprintf(stderr, "ERROR: File %s already not tracked and cannot be removed.\n", filename);
    return 1;
  }
  return 0;
}

/* Usage: `pit commit -m <message>`
 *
 * Creates new commit folder with a copy of all currently tracked files
 * and the current .index and .prev files.
 */
int pit_commit(const char *message) {
  // Checks if on a head
  char current_branch[BRANCHNAME_SIZE];
  fs_read_file(".pit/.current_branch", current_branch, BRANCHNAME_SIZE);

  if (strlen(current_branch) == 0) {
    fprintf(stderr, "ERROR: Need to be on HEAD of a branch to commit\n");
    return 1;
  }

  // Gets last id from .prev file and generates a new id
  char last_id[COMMIT_ID_BYTES + 1];
  fs_read_file(".pit/.prev", last_id, COMMIT_ID_BYTES + 1);
  
  char next_id[33];
  generate_random_id(next_id, 33);

  // Create new directory .pit/<new_id> and copy .index and .prev into this directory
  char *new_dir = concat_strings(2, ".pit/", next_id);
  fs_mkdir(new_dir);

  char *id_index = concat_strings(2, new_dir, "/.index");
  fs_cp(".pit/.index", id_index);
  free(id_index);

  char *id_prev = concat_strings(2, new_dir, "/.prev");
  fs_cp(".pit/.prev", id_prev);
  free(id_prev);

  // Copy files listed in .pit/.index to .pit/<new_id> directory
  FILE *findex = fs_open(".pit/.index", "r");
  char line[FILENAME_SIZE + 1];
  while (fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");

    char *dest_path = concat_strings(3, new_dir, "/", line);
    fs_cp(line, dest_path);
    free(dest_path);
  }
  fclose(findex);

  // Store the commit message in .pit/<new_id>/.message
  char *msg_path = concat_strings(2, new_dir, "/.message");
  free(new_dir);

  fs_write_file(msg_path, message);
  fs_write_file(".pit/.prev", next_id);
  free(msg_path);

  return 0;
}

/* Usage: `pit status`
 *
 * Prints all currently tracked files.
 */
int pit_status() {
  FILE *findex = fs_open(".pit/.index", "r");
  
  fprintf(stdout, "Tracked files:\n\n");

  int count = 0;
  char line[FILENAME_SIZE];
  while (fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");

    fprintf(stdout, "    %s\n", line);
    count++;
  }

  fprintf(stdout, "\n%d file(s) total.\n", count);
  return 0;
}

/* Usage: `pit log`
 * 
 * Prints all git commits in order of latest to earliest.
 */
int pit_log() {
  char last_id[COMMIT_ID_SIZE + 1];
  fs_read_file(".pit/.prev", last_id, COMMIT_ID_SIZE);

  char current_id[COMMIT_ID_SIZE + 1];
  while (strcmp(last_id, BASE_COMMIT_ID) != 0) {
    fprintf(stdout, "commit %s\n", last_id);

    char *message_path = concat_strings(3, ".pit/", last_id, "/.message");
    char current_message[MESSAGE_SIZE + 1];
    fs_read_file(message_path, current_message, MESSAGE_SIZE);
    free(message_path);

    fprintf(stdout, "    %s\n\n", current_message);

    char *prev_path = concat_strings(3, ".pit/", last_id, "/.prev");

    fs_read_file(prev_path, current_id, COMMIT_ID_SIZE);
    free(prev_path);

    strcpy(last_id, current_id);
  }
  return 0;
}

/* `pit branch`
 * 
 * Prints all branches in order of creation (oldest to newest). Adds a star on current branch.
 */
int pit_branch() {
  FILE *fbranch = fopen(".pit/.branches", "r");

  char current_branch[BRANCHNAME_SIZE];
  fs_read_file(".pit/.current_branch", current_branch, BRANCHNAME_SIZE);

  char line[BRANCHNAME_SIZE];
  while (fgets(line, sizeof(line), fbranch)) {
    strtok(line, "\n");

    char symbol = ' ';
    if (strcmp(current_branch, line) == 0) {
      symbol = '*';
    }
    fprintf(stdout, " %c %s\n", symbol, line);
  }
  return 0;
}


/* Returns the branch number or -1 if the branch doesn't exist. */
int get_branch_number(const char *branch_name) {
  FILE *fbranches = fs_open(".pit/.branches", "r");

  int counter = 0;
  char line[BRANCHNAME_SIZE];
  while(fgets(line, sizeof(line), fbranches)) {
    strtok(line, "\n");
    if (strcmp(line, branch_name) == 0) {
      fclose(fbranches);
      return counter;
    }
    counter++;
  }
  return -1;
}

/* Returns 1 if it does and 0 if not. */
int does_commit_exist(const char *commit_id) {
    DIR *dir = opendir(".pit");
    if (dir == NULL) {
        perror("Unable to open .pit directory");
        return 0; // or consider exiting with an error code depending on the context
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Check if it's a directory but not the "." or ".." directories
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            if (strcmp(entry->d_name, commit_id) == 0) {
                closedir(dir);
                return 1;
            }
        }
    }
    closedir(dir);
    return 0;
}

int create_branch_file() {
  char current_branch[BRANCHNAME_SIZE];
  fs_read_file(".pit/.current_branch", current_branch, BRANCHNAME_SIZE);

  if (strlen(current_branch)) {
    char *current_branch_file = concat_strings(2, ".pit/.branch_", current_branch);
    fs_cp(".pit/.prev", current_branch_file);
    free(current_branch_file);
  }
  return 0;
}

/* `pit checkout <commit_id>` */
int pit_checkout_commit(char *commit_id, int is_explicitly_called) {
  // set .pit/.index to be "" and write id into .prev
  if (strcmp(commit_id, BASE_COMMIT_ID) == 0) {
    fs_write_file(".pit/.index", "");
    fs_write_file(".pit/.prev", commit_id);
    return 0;
  }
  
  if (!does_commit_exist(commit_id)) {
    fprintf(stderr, "ERROR: Commit id %s does not exist", commit_id);
    return 1;
  }

  if (is_explicitly_called) {
    // set HEAD of current branch to be the latest commit
    create_branch_file();
    fs_write_file(".pit/.current_branch", "");
  }

  // delete all the tracked files
  FILE *findex = fs_open(".pit/.index", "r");

  char line_old[FILENAME_SIZE + 1];
  while (fgets(line_old, sizeof(line_old), findex)) {
    strtok(line_old, "\n");

    fs_rm(line_old);
  }
  fclose(findex);

  // copy .pit/<commit_id>/.index to .pit/.index
  char *fcommit_index = concat_strings(3, ".pit/", commit_id, "/.index");
  fs_cp(fcommit_index, ".pit/.index");
  free(fcommit_index);

  // for each f in .pit/.index, copy .pit/<commit_id>/<f> to current directory <f>
  FILE *past_files = fs_open(".pit/.index", "r");
  char line_new[FILENAME_SIZE + 1];
  while (fgets(line_new, sizeof(line_new), past_files)) {
    strtok(line_new, "\n");

    char *fcommit_file = concat_strings(4, ".pit/", commit_id, "/", line_new);
    fs_cp(fcommit_file, line_new);
    free(fcommit_file);
  }
  fclose(past_files);

  // write id of current commit to .pit/.prev
  fs_write_file(".pit/.prev", commit_id);
  return 0;
}

/* `pit checkout <branch>` */
int pit_checkout_branch(char *branch_name) {
  if (get_branch_number(branch_name) == -1) {
    fprintf(stderr, "ERROR: Branch name %s does not exist!", branch_name);
    return 1;
  }

  // set HEAD of current branch to be the latest commit
  create_branch_file();

  // Set current branch to be this branch and read HEAD of this branch
  char *branch_file = concat_strings(2, ".pit/.branch_", branch_name);
  fs_write_file(".pit/.current_branch", branch_name);
  char branch_head_commit_id[COMMIT_ID_SIZE + 1];
  fs_read_file(branch_file, branch_head_commit_id, COMMIT_ID_SIZE + 1);
  free(branch_file);

  // checkout the commit representing the head
  return pit_checkout_commit(branch_head_commit_id, 0);
}

/* `pit checkout -b <new_branch>` */
int pit_create_branch(char *new_branch) {
  if (get_branch_number(new_branch) != -1) {
    fprintf(stderr, "ERROR: Branch name %s already exists!", new_branch);
    return 1;
  }

  // set HEAD of current branch to be the latest commit
  create_branch_file();

  // add this branch to the list of branches and set HEAD of this branch to be
  // latest commit
  char *branch_file = concat_strings(2, ".pit/.branch_", new_branch);
  FILE *fbranches = fs_open(".pit/.branches", "a");
  fs_append_file(".pit/.branches", new_branch);
  fs_cp(".pit/.prev", branch_file);

  // Set current branch to be this branch and read HEAD of this branch
  fs_write_file(".pit/.current_branch", new_branch);
  char branch_head_commit_id[COMMIT_ID_SIZE + 1];
  fs_read_file(branch_file, branch_head_commit_id, COMMIT_ID_SIZE + 1);
  free(branch_file);

  // checkout the commit representing the head
  return pit_checkout_commit(branch_head_commit_id, 0);
}