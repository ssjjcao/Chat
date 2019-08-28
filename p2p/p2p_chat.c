#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

#define MAX_MSG_LEN 1024

struct msg{
    long type;
    char text[MAX_MSG_LEN];
};

int main(int argc, char *argv[]){
    int group;
    int index;
    key_t key;
    pid_t pid;
    int msg_id;
    long snd_type;
    long rcv_type;
    struct msg msg_data;
    char buf[MAX_MSG_LEN];
    if(argc != 3){
        fprintf(stderr, "command error!\ne.g.: ./p2p_chat 1 [group: >= 1] 1 [index: 1 or 2]\n");
        return -1;
    }

    group = atoi(argv[1]);
    index = atoi(argv[2]);
    key = ftok("./p2p_chat.c", 1);
    snd_type = group * 2 + index;
    rcv_type = index == 1 ? (group * 2 + 2) : (group * 2 + 1);

    if((msg_id = msgget(key, 0644 | IPC_CREAT)) == -1){
        fprintf(stderr, "msgget error!\n");
        exit(-1);
    }

    if((pid = fork()) == 0){
        while(1){
            if(msgrcv(msg_id, (void *)&msg_data, MAX_MSG_LEN, rcv_type, 0) == -1){
                pause(); //wait parent's signal SIGINT
            }
            printf("%s", msg_data.text);
            if(strncmp(msg_data.text, "stop chatting", 13) == 0){
                msgctl(msg_id, IPC_RMID, 0);
                break;
            }
        }
    }else{
        printf("you can start chatting with group %d, index %ld\n", group, 2 - rcv_type % 2);
        while(1){
            fgets(buf, MAX_MSG_LEN, stdin);
            msg_data.type = snd_type;
            strcpy(msg_data.text, buf);
            if(msgsnd(msg_id, (void *)&msg_data, MAX_MSG_LEN, 0) == -1){
                printf("%s\n", "chat is over, Ctrl+C to leave.");
            }
            if(strncmp(buf, "stop chatting", 13) == 0){
                if(kill(pid, SIGINT) == -1){
                    fprintf(stderr, "cannot send SIGINT to child\n");
                }
                sleep(1);
                break;
            }
        }
    }

    return 0;
}
