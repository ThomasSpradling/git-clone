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

  FILE* findex = fopen(".pit/.index", "w");
  fclose(findex);

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
int pit_add(const char *filanem) {
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