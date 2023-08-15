int pit_init();
int pit_add(const char *filename);
int pit_rm(const char *filename);
int pit_commit(const char *message);
int pit_status();
int pit_log();

#define COMMIT_ID_BYTES 40  // Number of bytes in a commit id

#define FILENAME_SIZE 512
#define COMMIT_ID_SIZE (COMMIT_ID_BYTES + 1)
#define MESSAGE_SIZE 512