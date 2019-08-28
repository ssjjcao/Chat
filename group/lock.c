#include <stdio.h>
#include <stdlib.h>
#include "lock.h"

void lock(int sem_id, int sem_num){
    struct sembuf wait = {sem_num, -1, SEM_UNDO};
    if(semop(sem_id, &wait, 1) == -1){
        fprintf(stderr, "semop error!\n");
        exit(-1);
    }
}

void unlock(int sem_id, int sem_num){
    struct sembuf release = {sem_num, 1, SEM_UNDO};
    if(semop(sem_id, &release, 1) == -1){
        fprintf(stderr, "semop error!\n");
        exit(-1);
    }
}
