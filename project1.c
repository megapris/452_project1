#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#define READ 0
#define WRITE 1
int fd[100][2];
int numProcesses;
int root;
int next;
int apple;
int myIndex;
char inbox[100];
int node;
bool badApple = false;

struct apple
{
    char message[100];
    int node;
};
void controlC(int sig)
{
    printf("Gracefully exiting...\n");
    exit(0);
}
void spawnProcesses(int count)
{
    pid_t pid;
    pid = getpid();
    if (count >= numProcesses)
    {
        return;
    }
    if ((pid = fork()) < 0)
    {
        printf("Error while forking");
    }
    else if (pid == 0)
    {
        myIndex = count;
        if (myIndex == numProcesses / 2)
        {
            badApple = true;
        }
        spawnProcesses(count + 1);
    }
    else
    {
        next = pid;
    }
}

void sendMessage(struct apple myApple)
{
    printf("Process %d is sending the message to Process %d...\n", myIndex, myIndex + 1 % numProcesses);
    write(fd[myIndex][1], &myApple, sizeof(char) * sizeof(myApple));
}

void readMessage()
{
    struct apple myApple;
    int i = myIndex == 0 ? numProcesses - 1 : myIndex - 1;
    read(fd[i][0], &myApple, sizeof(char) * sizeof(myApple));
    printf("Process %d recieved the message\n\n", myIndex);
    if (badApple)
    {
        int myRand = rand() % 10000 + 500;
        char message[100];
        sprintf(message, "bad_apple%d", myRand);
        strcpy(myApple.message, message);
    }
    if (myIndex == myApple.node)
    {
        strcpy(inbox, myApple.message);
        printf("Process %d is the recipient\n", myIndex);
        printf("The message is: %s\n", inbox);
        strcpy(myApple.message, "");
    }
    else
    {
        // printf("sleeping... \n");
        sleep(1);
    }
    if (myIndex != 0)
    {
        sendMessage(myApple);
    }
}

void createPipes(int num)
{
    int pipeResult;
    for (int i = 0; i < num; i++)
    {
        if ((pipeResult = pipe(fd[i])) < 0)
        {
            printf("ERROR WHILE PIPING");
        }
    }
}

int main()
{
    signal(SIGINT, controlC);

    char string1[100];
    char string2[100];
    char string3[100];
    int number1, number2, number3;

    struct apple myApple;

    printf("How many processes: ");
    fgets(string1, 100, stdin);
    string1[strlen(string1) - 1] = '\0';
    numProcesses = atoi(string1);

    createPipes(numProcesses);
    spawnProcesses(1);
    int pid = getpid();

    char temp[100];
    while (1)
    {
        if (myIndex == 0)
        {

            printf("What is your message: ");
            fgets(string2, 100, stdin);
            string2[strlen(string2) - 1] = '\0';
            strcpy(myApple.message, string2);

            printf("Which node would you like to recieve the message: ");
            fgets(string3, 100, stdin);
            string3[strlen(string3) - 1] = '\0';
            myApple.node = atoi(string3);
            if(myApple.node >= numProcesses){
                printf("Invalid node number\n");
                exit(0);
            }
            printf("\n");

            sendMessage(myApple);
        }
        readMessage();
    }
    // printf("PROCESS %d EXITING\n", myIndex);

    return 0;
}
