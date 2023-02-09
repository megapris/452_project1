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

struct apple{
    char message[100];
    int node;
};

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

void sendMessage(struct apple myApple){
    printf("Process %d is sending the message to Process %d...\n", index, index + 1 % numProcesses);
    write(fd[index][1], &myApple, sizeof(char) * sizeof(myApple));
}

void readMessage(){
    struct apple myApple;
    char message[100];
    int i = index == 0 ? numProcesses - 1 : index - 1;
    read(fd[i][0], &myApple, sizeof(char) * sizeof(myApple));
    printf("Process %d rercieved the message\n", index);
    if(index == myApple.node){
        printf("The message is: %s\n", myApple.message);
        strcpy(myApple.message, "");
    }
    else{
        // printf("sleeping... \n");
        sleep(1);
    }
    sendMessage(myApple);
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
    struct apple myApple;
    printf("How many processes: \n");
    scanf("%d", &numProcesses);
    createPipes(numProcesses);
    spawnProcesses(1);
    int pid = getpid();
    while(1){
        if(index == 0){
            printf("What is your message: ");
            scanf("%s", &myApple.message);
            printf("Which node would you like to recieve the message: ");
            scanf("%d", &myApple.node);
            sendMessage(myApple);
        }
        readMessage();
    }
    // printf("PROCESS %d EXITING\n", index);
}