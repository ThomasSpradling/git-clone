#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <uuid/uuid.h>
#include "pit.h"

void generate_random_id(char* buffer, size_t size) {
  uuid_t bin_uuid;

  // Ensure the provided buffer has enough space
  if (size < 33) {
    fprintf(stderr, "ERROR: Buffer not large enough for UUID");
    exit(1);
  }

  uuid_generate(bin_uuid);

  snprintf(buffer, 33, "%08x%04x%04x%04x%012x", 
    *((unsigned int*)(bin_uuid)),
    *((unsigned int*)(bin_uuid + 4)),
    *((unsigned int*)(bin_uuid + 6)),
    *((unsigned int*)(bin_uuid + 8)),
    *((unsigned long*)(bin_uuid + 10))
  );
}

char* concat_strings(int count, ...) {
  va_list args;
  int total_length = 0;

  // First, compute the total length required for the concatenated string
  va_start(args, count);
  for (int i = 0; i < count; i++) {
    total_length += strlen(va_arg(args, char*));
  }
  va_end(args);

  // Allocate memory for the result (plus one for the null terminator)
  char* result = malloc(total_length + 1);
  if (!result) {
    perror("Memory allocation failed");
    exit(1);
  }
  result[0] = '\0'; // Start with an empty string

  // Then, concatenate each string
  va_start(args, count);
  for (int i = 0; i < count; i++) {
    strcat(result, va_arg(args, char*));
  }
  va_end(args);

  return result;
}

// File system helper methods

/* Opens file with error handling. */
static FILE *fs_open(const char *filename, const char *mode) {
  if (filename == NULL || strlen(filename) > FILENAME_SIZE) {
    fprintf(stderr, "ERROR: Invalid file name!\n");
    exit(1);
  }
  FILE *findex = fopen(filename, mode);
  if (!findex) {
    fprintf(stderr, "ERROR: Could not open file %s!\n", filename);
    exit(1);
  }
  return findex;
}

/* Makes a directory under name `dirname`. */
static void fs_mkdir(const char *dirname) {
  if (dirname == NULL || strlen(dirname) > FILENAME_SIZE) {
    fprintf(stderr, "ERROR: Invalid file name!\n");
    exit(1);
  }
  if(mkdir(dirname, 0777)) {
    fprintf(stderr, "ERROR: Creating directory %s failed.\n", dirname);
    exit(1);
  }
}

/* If string is not null, it replaces contents of `filename` with that string.
 * Creates file if necessary.
 */
static void fs_write_file(const char *filename, const char *string) {
  FILE * findex = fs_open(filename, "w");
  if (string) {
    fprintf(findex, "%s", string);
  }
  fclose(findex);
}

/* Reads entire file and returns it as a string. */
void fs_read_file(char *filename, char *str, int size) {
  FILE *file = fs_open(filename, "r");

  size_t actual_size = fread(str, 1, size, file);
  str[actual_size] = '\0';
  
  fclose(file);
}

/* Removes file with name `filename`. */
static void fs_rm(const char *filename) {
  if (filename == NULL || strlen(filename) > FILENAME_SIZE) {
    fprintf(stderr, "ERROR: Invalid file name!\n");
    exit(1);
  }
  if (unlink(filename)) {
    fprintf(stderr, "ERROR: Could not delete file %s!\n", filename);
    exit(1);
  }
}

/* Moves file from src to dest. */
static void fs_mv(const char *src, const char *dest) {
  if (!src || !dest || strlen(src) > FILENAME_SIZE || strlen(dest) > FILENAME_SIZE) {
    fprintf(stderr, "ERROR: Invalid file name(s)!\n");
    exit(1);
  }
  if (rename(".pit/.newindex", ".pit/.index")) {
    fprintf(stderr, "ERROR: Failed to rename file from %s to %s!\n", src, dest);
    exit(1);
  }
}

static void ensure_directory(const char *path) {
  char tmp[1024];
  char *p = NULL;

  snprintf(tmp, sizeof(tmp), "%s", path);
  for (p = tmp + 1; *p; p++) {
    if (*p == '/') {
        *p = 0;
        mkdir(tmp, 0777);
        *p = '/';
    }
  }
}

/* Copies file from src to dest. */
static void fs_cp(const char *src, const char *dest) {
  char buffer[1024];
  size_t bytes;

  ensure_directory(dest);  // Ensure all directories in the path exist

  FILE *fsrc = fs_open(src, "r");
  FILE *fdest = fs_open(dest, "w");

  while ((bytes = fread(buffer, 1, sizeof(buffer), fsrc)) > 0) {
      fwrite(buffer, 1, bytes, fdest);
  }

  fclose(fsrc);
  fclose(fdest);
}

/* Append string to end of file */
static void fs_append_file(const char *filename, const char *string) {
  FILE * findex = fs_open(filename, "a");
  if (string) {
    fprintf(findex, "%s\n", string);
  }
  fclose(findex);
}