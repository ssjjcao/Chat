#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include "lock.h"

#define SHM_SIZE 1024
#define NAME_SIZE 32    
#define EMPTY 0
#define FULL 1
#define TEMPFILE "./group_members.txt"

void *shm_addr;

void sig_usr(int);

int main(int argc, char *argv[]){
    char *name;
    char my_pid[64];
    key_t sem_key = ftok("./group_master.c", 1);
    key_t shm_key = ftok("./group_master.c", 2);
    int sem_id = semget(sem_key, 2, 0644);
    int shm_id = shmget(shm_key, SHM_SIZE, 0644);
    if((shm_addr = shmat(shm_id, 0, 0)) == (void *)-1){
        fprintf(stderr, "shmat error!\n");
        exit(-1);
    }

    if(argc != 2){
        fprintf(stderr, "command error! e.g.: ./group_chat zhangsan [name]\n");
        return -1;
    }
    name = argv[1];
    
    printf("%s\n", "you need to input \"send\" before you send message");
    
    FILE *f = fopen(TEMPFILE, "a");
    sprintf(my_pid, "%d", (int)getpid());
    fputs(my_pid, f);
    fputs("\n", f);
    fclose(f);
    
    signal(SIGUSR1, sig_usr);

    while(1){
        char instruction_buf[128];
        scanf("%s", instruction_buf);
        if(strcmp(instruction_buf, "send") == 0){
            char message_buf[SHM_SIZE - NAME_SIZE];
            lock(sem_id, EMPTY);
            printf("%s\n", "enter message: ");
            fgets(message_buf, SHM_SIZE - NAME_SIZE, stdin);
            fgets(message_buf, SHM_SIZE - NAME_SIZE, stdin);
            char message_to_send[SHM_SIZE];
            strcpy(message_to_send, name);
            strcat(message_to_send, ": ");
            strcat(message_to_send, message_buf);
            memcpy(shm_addr, message_to_send, SHM_SIZE);
            unlock(sem_id, FULL);
        }else{
            printf("%s\n", "unknown instruction");
        }
    }    
}

void sig_usr(int signo){
    printf("receive message from %s", (char *)shm_addr);
}