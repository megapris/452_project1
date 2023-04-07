#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>

int mixer;
int pantry;
int refrigerator;
int bowl;
int spoon;
int oven; 

struct sembuf p = { 0, -1, SEM_UNDO};
struct sembuf v = { 0, +1, SEM_UNDO};

typedef struct{
    int itemID;
    char name[10];
}Item;

typedef struct{
    int num;
    bool badChef;
    bool ramsied;
    bool mixer;
    bool spoon;
    bool bowl;
    bool oven;
    Item targetItem;
}Baker;

void cook(Baker *baker, Item targetItem);

const Item cookies = {0, "cookies"};
const Item pancakes = {1,"pancakes"};
const Item dough = {2,"dough"};
const Item pretzels = {3,"pretzels"};
const Item rolls = {4,"rolls"};
const Item cookBook[] = {cookies, pancakes, dough, pretzels, rolls};

void swap(Item *a, Item *b) {
    Item temp = *a;
    *a = *b;
    *b = temp;
}

void scrambleArray(Item *arr, int size, int offset) {
    srand(time(NULL) + offset);
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        swap(&arr[i], &arr[j]);
    }
}

void fetchItem(int id, bool pantry, Item item){
    if(pantry){
        printf("THREAD %d WAITING FOR PANTRY\n", id);
        if (semop(pantry, &p, 1) == -1) {
            perror("semop (p): semop failed");
            exit(1);
        }
        printf("THREAD %d FETCHING FROM PANTRY\n", id);
        sleep(3);
        if (semop(pantry, &v, 1) == -1) {
            perror("semop (v): semop failed");
            exit(1);
        }
    }
    else{
        printf("THREAD %d WAITING FOR REFRRIGERATOR\n", id);
        if (semop(refrigerator, &p, 1) == -1) {
            perror("semop (p): semop failed");
            exit(1);
        }
        printf("THREAD %d FETCHING FROM REFRRIGERATOR\n", id);
        sleep(3);
        if (semop(refrigerator, &v, 1) == -1) {
            perror("semop (v): semop failed");
            exit(1);
        }
    }
}

void releaseSemaphore(int sem, Baker *b, int id){
    char *appliance;
    switch(id){
        case 0:
            appliance = "SPOON";
            b->spoon = false;
            break;
        case 1:
            appliance = "MIXER";
            b->mixer = false;
            break;
        case 2:
            appliance = "BOWL";
            b->bowl = false;
            break;
        case 3:
            appliance = "OVEN";
            b->oven = false;
            break;
    }
    if (semop(sem, &v, 1) == -1) {
        perror("semop (v): semop failed");
        exit(1);
    }
    printf("THREAD %d RELEASED %s\n", b->num, appliance);
}

void inspect(Baker *b){
    srand(time(NULL) + b->num);
    if(rand()%2 == 0){
        printf("THREAD %d HAS BEEN RAMSIED\n", b->num);
        if(b->spoon){
            releaseSemaphore(spoon, b, 0);
        }
        if(b->mixer){
            releaseSemaphore(mixer, b, 1);
        }
        if(b->bowl){
            releaseSemaphore(bowl, b, 2);
        }
        if(b->oven){
            releaseSemaphore(oven, b, 3);
        }
        b->ramsied = true;
        cook(b, b->targetItem);
    }
}

void waitSemaphore(int sem, Baker *b, int id){
    char *appliance;
    switch(id){
        case 0:
            appliance = "SPOON";
            b->spoon = true;
            break;
        case 1:
            appliance = "MIXER";
            b->mixer = true;
            break;
        case 2:
            appliance = "BOWL";
            b->bowl = true;
            break;
        case 3:
            appliance = "OVEN";
            b->oven = true;
            break;
    }
    printf("THREAD %d FETCHING %s\n", b->num, appliance);
    if(semop(sem, &p, 1) == -1) {
            perror("semop (p): semop failed");
            exit(1);
    }
    if(b->badChef && !b->ramsied){inspect(b);}    
}

void fetchAppliance(Baker *b){
    waitSemaphore(spoon, b, 0);
    waitSemaphore(mixer, b, 1);
    waitSemaphore(bowl, b, 2);
    waitSemaphore(oven, b, 3);
    printf("THREAD %d COOKING FOOD\n", b->num);
    sleep(5);
    printf("THREAD %d RELEASING APPLIANCES\n", b->num);
    releaseSemaphore(spoon, b, 0);
    releaseSemaphore(bowl, b, 2);
    releaseSemaphore(mixer, b, 1);
    releaseSemaphore(oven, b, 3);
}

void cook(Baker *baker, Item targetItem){
    bool visitPantry = rand() % 2;
    fetchItem(baker->num, visitPantry, targetItem);
    fetchItem(baker->num, !visitPantry, targetItem);
    fetchAppliance(baker);
}

void * bakerFunc(void* arg){  
    Baker* baker = (Baker *)arg;
    srand(time(NULL) + baker->num);
    Item allItems[5];
    for(int i = 0; i < 5; i++){
        allItems[i] = cookBook[i];
    }
    scrambleArray(allItems,5, baker->num);
    Item targetItem;
    for(int i = 0; i < 5; i++){
        baker->targetItem = allItems[i];
        printf("THREAD %d is making %s\n", baker->num, baker->targetItem.name);
        cook(baker, targetItem);
    }
    free(baker);
}

void clearArray(int arr[], int size){
    for(int i = 0; i < size; i++){
        arr[i] = 0;
    }
}

void initializeSemaphores(){
    if ((mixer = semget(IPC_PRIVATE, 1, S_IRUSR | S_IWUSR)) == -1) {
        perror("semget: semget failed");
        exit(1);
    }
    semctl(mixer, 0, SETVAL, 2);
    if ((pantry = semget(IPC_PRIVATE, 1, S_IRUSR | S_IWUSR)) == -1) {
        perror("semget: semget failed");
        exit(1);
    }
    semctl(pantry, 0, SETVAL, 1);
    if ((refrigerator = semget(IPC_PRIVATE, 1, S_IRUSR | S_IWUSR)) == -1) {
        perror("semget: semget failed");
        exit(1);
    }
    semctl(refrigerator, 0, SETVAL, 2);
    if ((bowl = semget(IPC_PRIVATE, 1, S_IRUSR | S_IWUSR)) == -1) {
        perror("semget: semget failed");
        exit(1);
    }
    semctl(bowl, 0, SETVAL, 3);

    if((spoon = semget(IPC_PRIVATE, 1, S_IRUSR | S_IWUSR)) == -1) {
        perror("semget: semget failed");
        exit(1);
    }
    semctl(spoon, 0, SETVAL, 5);
    if ((oven = semget(IPC_PRIVATE, 1, S_IRUSR | S_IWUSR)) == -1) {
        perror("semget: semget failed");
        exit(1);
    }
    semctl(oven, 0, SETVAL, 1);
}

void main(){
    int numBakers;
    initializeSemaphores();
    printf("How many bakers\n");
    scanf("%d", &numBakers);
    pthread_t threads[numBakers];
    pthread_attr_t attributes[numBakers];
    srand(time(NULL));
    int badChef = rand()%numBakers;
    for(int i = 0; i < numBakers; i++){
        Baker* b = (Baker*)malloc(sizeof(Baker));
        b->num = i;
        b->badChef = i == badChef;
        b->oven = false;
        b->bowl = false;
        b->spoon = false;
        b->mixer = false;
        b->ramsied = false;
        b->targetItem = cookies;
        pthread_create (&threads[i], NULL, bakerFunc, b);
    }
    int joinReturnValue;
    for(int i = 0; i < numBakers; i++){
        pthread_join (threads[i], (void **) &joinReturnValue);    
    }
}