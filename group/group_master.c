#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include "lock.h"

#define SHM_SIZE 1024
#define EMPTY 0
#define FULL 1
#define TEMPFILE "./group_members.txt"

union semun{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int sem_id, shm_id;

void exit_group(int);
void notify(void);

int main(int argc, char *argv[]){
    key_t sem_key = ftok("./group_master.c", 1);
    key_t shm_key = ftok("./group_master.c", 2);
    sem_id = semget(sem_key, 2, 0644 | IPC_CREAT);
    shm_id = shmget(shm_key, SHM_SIZE, 0644 | IPC_CREAT);
    union semun sem_init;
    void *shm_addr;

    if((shm_addr = shmat(shm_id, 0, 0)) == (void *)-1){
        fprintf(stderr, "shmat error!\n");
        exit(-1);
    }

    sem_init.val = 1;
    semctl(sem_id, EMPTY, SETVAL, sem_init);
    sem_init.val = 0;
    semctl(sem_id, FULL, SETVAL, sem_init);

    signal(SIGINT, exit_group);
    printf("%s\n%s\n", "group chat room inits ok, you can start chatting by ./group_chat [your name]!", "Ctrl+C to exit.");
    printf("\n%s\n", "chat record:");

    while(1){
        lock(sem_id, FULL);
        printf("%s", (char *)shm_addr);
        notify();
        unlock(sem_id, EMPTY);
    }
}

void exit_group(int signo){
    if(access(TEMPFILE, F_OK) == 0){
        remove(TEMPFILE);
    }
    semctl(sem_id, EMPTY, IPC_RMID);
    semctl(sem_id, FULL, IPC_RMID);
    if(shmctl(shm_id, IPC_RMID, NULL) == 0){
        printf("\n%s\n", "group exits");
        exit(0);
    }
}

void notify(){
    char buf[64];
    FILE *f = fopen(TEMPFILE, "r");
    while(fgets(buf, 64, f) != NULL){
        if(strlen(buf) > 1){
            kill(atoi(buf), SIGUSR1);
        }
    }
    fclose(f);
}