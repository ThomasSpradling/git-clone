#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "pit.h"
#include "util.h"

/* `pit init`
 * Creates a `.pit` directory with created `.index` and `.prev` files
 */
int pit_init() {
  int result = mkdir(".pit", 0777);
  if (result != 0) {
    fprintf(stderr, "Creating directory `.pit` failed");
    exit(1);
  }

  // opens .index and .prev files
  FILE* findex = fopen(".pit/.index", "w");
  FILE* fprev = fopen(".pit/.prev", "w");
  if (findex == NULL || fprev == NULL) {
    fprintf(stderr, "Could not open necessary files.");
  }

  // adds id of first commit to `.prev`
  const char *id = "0000000000000000000000000000000000000000";
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
      return 3;
    }

    fprintf(fnewindex, "%s\n", line);
  }
  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);
  
  if (rename(".pit/.newindex", ".pit/.index")) {
    fprintf(stderr, "ERROR: Failed to rename file!\n");
    return 3;
  }
  return 0;
}

/* pit rm <filename>
 * 
 */
int pit_rm(const char *filename) {
  return 0;
}

/* pit commit -m <message>
 * 
 */
int pit_commit(const char *message) {
  return 0;
}

/* pit status
 * 
 */
int pit_status() {
  return 0;
}

/* pit log
 * 
 */
int pit_log() {
  return 0;
}