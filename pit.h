int pit_init();
int pit_add(const char *filename);
int pit_rm(const char *filename);
int pit_commit(const char *message);
int pit_status();
int pit_log();
int pit_branch();

#define COMMIT_ID_BYTES 36
#define BASE_COMMIT_ID "00000000000000000000000000000000"
#define COMMIT_ID_SIZE (COMMIT_ID_BYTES + 1)

#define FILENAME_SIZE 512
#define MESSAGE_SIZE 512

#define BRANCHNAME_SIZE 128
#define COMMIT_ID_BRANCH_BYTES 10