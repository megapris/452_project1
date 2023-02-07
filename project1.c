#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define READ 0
#define WRITE 1
int fd[100][2];
int numProcesses;
int root;
int next;
int apple;
int index;
char inbox[100];
int node;

void spawnProcesses(int count){
    pid_t pid;
    pid = getpid();
    if(count >= numProcesses){ return; }
    if((pid = fork()) < 0){
        printf("Error while forking");
    }
    else if(pid == 0){
        index = count;
        spawnProcesses(count + 1);
    }
    else{
        next = pid;
    }
}

void sendMessage(char message[100]){
    printf("Process %d is sending the message to Process %d...\n", index, index + 1 % numProcesses);
    write(fd[index][1], &message, sizeof(char) * sizeof(message));
}

void readMessage(){
    char message[100];
    int i = index == 0 ? numProcesses - 1 : index - 1;
    // printf("Process %d is reading the message from Process %d...\n", index, i);
    read(fd[i][0], &message, sizeof(char) * sizeof(char) * 100);
    if(index == node){
        // inbox = message;
        printf("MESSAGE HAS BEEN RESEVED\n");
    }
    else{
        printf("sleeping... \n");
        sleep(1);
        sendMessage(message);
    }
}

void createPipes(int num){
    int pipeResult;
    for(int i = 0; i < num; i++){
        if((pipeResult = pipe(fd[i])) < 0){
            printf("ERROR WHILE PIPING");
        }
    }
}

void main(){
    char message[100];
    printf("How many processes: \n");
    scanf("%d", &numProcesses);
    createPipes(numProcesses);
    spawnProcesses(1);
    // wait();
    int pid = getpid();
    if(apple == index){
        printf("What is your message: ");
        scanf("%s", &message);
        printf("Which node would you like to recieve the message: ");
        scanf("%d", &node);
        sendMessage(message);
    }
    readMessage();
    printf("PROCESS %d EXITING\n", index);
}
