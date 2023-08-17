int pit_init();
int pit_add(const char *filename);
int pit_rm(const char *filename);
int pit_commit(const char *message);
int pit_status();
int pit_log();
int pit_branch();
int pit_checkout_commit(char *commit_id, int is_explicitly_called);
int pit_checkout_branch(char *branch_name);
int pit_create_branch(char *new_branch);


#define COMMIT_ID_SIZE 32
#define BASE_COMMIT_ID "00000000000000000000000000000000"
#define COMMIT_ID_BYTES (COMMIT_ID_SIZE + 1)

#define FILENAME_SIZE 512
#define MESSAGE_SIZE 512

#define BRANCHNAME_SIZE 10
#define COMMIT_ID_BRANCH_BYTES 10