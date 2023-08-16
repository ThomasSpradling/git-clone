#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <uuid/uuid.h>

char* generate_random_id() {
    uuid_t bin_uuid;
    char* buffer = (char*) malloc(33);  // 32 characters for UUID + null terminator

    if (!buffer) {
        perror("Memory allocation failed for UUID");
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

    return buffer;
}

int copy_file(const char *source_path, const char *dest_path) {
    char buffer[1024];
    size_t bytes;

    FILE *source = fopen(source_path, "r");
    FILE *dest = fopen(dest_path, "w");

    if (!source || !dest) {
        fprintf(stderr, "ERROR: Cannot copy files!");
        return 1;
    }

    while ((bytes = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        fwrite(buffer, 1, bytes, dest);
    }

    fclose(source);
    fclose(dest);
    return 0;
}

void _read_string(char *filename, char *str, int size) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "ERROR: Cannot view commit history!");
    exit(1);
  }
  
  // Read up to 'size' characters or until EOF, whichever comes first
  size_t actual_size = fread(str, 1, size, file);
  str[actual_size] = '\0';  // Properly null-terminate the string
  
  fclose(file);
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