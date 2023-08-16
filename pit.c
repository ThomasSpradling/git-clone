#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "pit.h"
#include "utils.h"

/* `pit init`
 * Creates a `.pit` directory with created `.index` and `.prev` files
 */
int pit_init() {
  int result = mkdir(".pit", 0777);
  if (result != 0) {
    fprintf(stderr, "Creating directory `.pit` failed");
    return 1;
  }

  // opens .index and .prev files
  FILE* findex = fopen(".pit/.index", "w");
  FILE* fprev = fopen(".pit/.prev", "w");
  if (findex == NULL || fprev == NULL) {
    fprintf(stderr, "Could not open necessary files.");
  }

  // adds id of first commit to `.prev`
  const char *id = "00000000000000000000000000000000";
  fwrite(id, 1, strlen(id) + 1, fprev);

  // closes files
  fclose(findex);
  fclose(fprev);
  return 0;
}

/* `pit add <filename>`
 *
 * Appends filename to list in `.index`
 * 
 * Errors:
 * >> ERROR: File <filename> already added
 * 
 */
int pit_add(const char *filename) {
  // Creates a newindex to later copy into for atomicity purposes
  FILE *findex = fopen(".pit/.index", "r");
  FILE *fnewindex = fopen(".pit/.newindex", "w");

  if (findex == NULL || fnewindex == NULL) {
    fprintf(stderr, "Could not open necessary files.");
    return 1;
  }

  // goes line-by-line through .index
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n"); // remove \n from the line

    // if we've seen this file before, stop process and throw error
    if (strcmp(line, filename) == 0) {
      fprintf(stderr, "ERROR: File %s already added!\n", filename);
      
      fclose(findex);
      fclose(fnewindex);
      // delete .newindex
      if (unlink(".pit/.newindex")) {
        fprintf(stderr, "ERROR: Failed to unlink file!\n");
      }
      return 1;
    }

    fprintf(fnewindex, "%s\n", line);
  }
  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);
  
  if (rename(".pit/.newindex", ".pit/.index")) {
    fprintf(stderr, "ERROR: Failed to rename file!\n");
    return 1;
  }
  return 0;
}

/* pit rm <filename>
 * Removes the specified file from `.index`
 */
int pit_rm(const char *filename) {
  FILE *findex = fopen(".pit/.index", "r");
  FILE *fnewindex = fopen(".pit/.newindex", "w");

  int has_found_value = 0;
  char line[FILENAME_SIZE];
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

  if (rename(".pit/.newindex", ".pit/.index")) {
    fprintf(stderr, "ERROR: Failed to rename file!\n");
    return 1;
  }

  if (!has_found_value) {
    fprintf(stderr, "ERROR: File already not tracked and cannot be removed.\n");
    return 1;
  }
  return 0;
}

/* pit commit -m <message>
 * Copies all tracked files into a commit folder along with .prev and .index data
 */
int pit_commit(const char *message) {
  char last_id[COMMIT_ID_BYTES + 1];
  _read_string(".pit/.prev", last_id, COMMIT_ID_BYTES);
  last_id[COMMIT_ID_BYTES] = '\0';
  
  char *next_id = generate_random_id();

  // Create new directory .pit/<new_id>
  char new_dir[COMMIT_ID_SIZE + 5];
  strcpy(new_dir, ".pit/");
  strcat(new_dir, next_id);
  int result = mkdir(new_dir, 0777);

  if (result != 0) {
    fprintf(stderr, "Creating directory in `.pit` failed");
    return 1;
  }

  // Copy .pit/.index to .pit/<new_id>/.index
  char dest_index[COMMIT_ID_SIZE + 12];
  strcpy(dest_index, new_dir);
  strcat(dest_index, "/.index");
  copy_file(".pit/.index", dest_index);

  // Copy .pit/.prev to .pit/<new_id>/.prev
  char dest_prev[COMMIT_ID_SIZE + 11];
  strcpy(dest_prev, new_dir);
  strcat(dest_prev, "/.prev");
  copy_file(".pit/.prev", dest_prev);

  // Copy files listed in .pit/.index to .pit/<new_id> directory
  FILE *findex = fopen(".pit/.index", "r");
  char line[FILENAME_SIZE];
  while (fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    char dest_path[COMMIT_ID_SIZE + FILENAME_SIZE + 10];
    strcpy(dest_path, new_dir);
    strcat(dest_path, "/");
    strcat(dest_path, line);
    copy_file(line, dest_path);
  }
  fclose(findex);

  // Store the commit message in .pit/<new_id>/.message
  char msg_path[COMMIT_ID_SIZE + 14];
  strcpy(msg_path, new_dir);
  strcat(msg_path, "/.message");
  FILE *fmsg = fopen(msg_path, "w");
  if (!fmsg) {
    perror("Error creating .message file");
    free(next_id);
    return 1;
  }
  fprintf(fmsg, "%s", message);
  fclose(fmsg);

  // Overwrite the .pit/.prev with the new id
  FILE *fprev_overwrite = fopen(".pit/.prev", "w");
  if (!fprev_overwrite) {
    fprintf(stderr, "Error opening .prev for overwriting");
    free(next_id);
    return 1;
  }
  fprintf(fprev_overwrite, "%s", next_id);

  // clean up
  free(next_id);

  return 0;
}

/* pit status
 * Goes through .index to print each file
 */
int pit_status() {
  FILE *findex = fopen(".pit/.index", "r");
  if (findex == NULL) {
    fprintf(stderr, "Could not open necessary files.");
    return 1;
  }
  
  fprintf(stdout, "Tracked files:\n\n");

  int count = 0;
  char line[FILENAME_SIZE];
  while (fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    fprintf(stdout, "\t%s\n", line);
    count++;
  }

  fprintf(stdout, "\n%d file(s) total.\n", count);
  return 0;
}

int pit_log() {
  char last_id[COMMIT_ID_SIZE + 1];
  _read_string(".pit/.prev", last_id, COMMIT_ID_SIZE);

  char current_id[COMMIT_ID_SIZE + 1] = "";  // Initialize it with an empty string

  while (strcmp(last_id, "00000000000000000000000000000000") != 0) {
    // Print the commit's ID
    printf("commit %s\n", last_id);

    // Compute and read the message associated with that ID
    char message_path[15 + COMMIT_ID_SIZE];
    strcpy(message_path, ".pit/");
    strcat(message_path, last_id);
    strcat(message_path, "/.message");

    char current_message[MESSAGE_SIZE + 1];
    _read_string(message_path, current_message, MESSAGE_SIZE);
    printf("\t%s\n\n", current_message);  // Print the commit message

    // Compute the path to the previous commit's ID
    char prev_path[5 + 6 + COMMIT_ID_SIZE + 1];
    strcpy(prev_path, ".pit/");
    strcat(prev_path, last_id);
    strcat(prev_path, "/.prev");

    if (access(prev_path, F_OK) == -1) {
        // The .prev file doesn't exist, which means we've reached the initial commit
        break;
    }

    _read_string(prev_path, current_id, COMMIT_ID_SIZE);
    strcpy(last_id, current_id);  // Update last_id for the next iteration
  }
  return 0;
}