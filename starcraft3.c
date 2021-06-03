//------------------------------------------------------------------------
// NAME: Nikolay Dinkov
// CLASS: XIv
// NUMBER: 16
// PROBLEM: starcraft3
// FILE NAME: starcraft3.c (unix file name)
// FILE PURPOSE:
// Имплементация на задачата starcraft3
//------------------------------------------------------------------------
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

struct MineralBlock
{
    int minerals;
    pthread_mutex_t mutex;
} *mineralBlock;

struct SCVs
{
    pthread_t *scv;
    int scv_number;
    int *scv_index;
} scv_all;

struct CommandCenter
{
    int mineralBlocks_number;
    int minerals;
    pthread_mutex_t commandCenter_mutex;
    pthread_t commandCenter_thread;
    int knights_number;
} commandCenter;

//------------------------------------------------------------------------
// FUNCTION: setupIndex (име на функцията)
// Функцията служи за инициализиране на масива, който държи индекса 
// на всеки един работник
// PARAMETERS:
// arr - масива с индексите
// n - поредния елемент, до който трябва да се слага индекс/бройката
// на работниците
//------------------------------------------------------------------------
void setupIndex(int arr[], int n)
{
    for(int i = 0; i < n; i++)
    {
        arr[i] = i + 1;
    }
}

//------------------------------------------------------------------------
// FUNCTION: scv_mine (име на функцията)
// Функцията служи за изкопаване, транспортиране и освобождаване
// на материали 
// PARAMETERS:
// p - по конвеция параметър, служи, за да се знае текущия 
// работник като се принтира
//------------------------------------------------------------------------
void *scv_mine(void *p)
{
    int *present_scv = (int*) p;
    int excavated_minerals;
    int *empty_mineralBlocks = malloc(sizeof(int) * commandCenter.mineralBlocks_number);
    int empty_number = 0;

    for(int i = 0; i < commandCenter.mineralBlocks_number; i++)
    {
        empty_mineralBlocks[i] = 1;
    }
    while(1)
    {
        for(int i = 0; i < commandCenter.mineralBlocks_number; i++)
        {
            if(empty_mineralBlocks[i] == 1)
            {
                sleep(3);
                if(pthread_mutex_trylock(&mineralBlock[i].mutex) == 0)
                {
                    if(mineralBlock[i].minerals == 0)
                    {
                        pthread_mutex_unlock(&mineralBlock[i].mutex);
                        empty_mineralBlocks[i] = 0;
                        empty_number++;
                        if(empty_number == commandCenter.mineralBlocks_number)
                        {
                            free(empty_mineralBlocks);
                            return NULL;
                        }
                        continue;
                    }
                    printf("SCV %d is mining from mineral block %d\n", *present_scv, i + 1);
                    if(mineralBlock[i].minerals < 8)
                    {
                        excavated_minerals = mineralBlock[i].minerals;
                        mineralBlock[i].minerals = 0;
                    } else {
                        mineralBlock[i].minerals -= 8;
                        excavated_minerals = 8;
                    }
                    pthread_mutex_unlock(&mineralBlock[i].mutex);
                    sleep(2);
                    printf("SCV %d is transporting minerals\n", *present_scv);
                    pthread_mutex_lock(&commandCenter.commandCenter_mutex);
                    commandCenter.minerals += excavated_minerals;
                    pthread_mutex_unlock(&commandCenter.commandCenter_mutex);
                    printf("SCV %d delivered minerals to the Command center\n", *present_scv);
                }
            }
        }
    }
}

//------------------------------------------------------------------------
// FUNCTION: commandCenter_work (име на функцията)
// Функцията служи за обучение на нови войници или създаване
// на нови работници 
// PARAMETERS:
// p - по конвеция параметър, не се използва
//------------------------------------------------------------------------
void *commandCenter_work(void *p)
{
    char c;
    while(1)
    {
        scanf("%c", &c);
        if(c == 'm')
        {
            pthread_mutex_lock(&commandCenter.commandCenter_mutex); 
            if(commandCenter.minerals >= 50)
            {
                commandCenter.minerals -= 50;
                commandCenter.knights_number += 1;
                pthread_mutex_unlock(&commandCenter.commandCenter_mutex);
                sleep(1);
                printf("You wanna piece of me, boy?\n");
            } else {
                pthread_mutex_unlock(&commandCenter.commandCenter_mutex);
                printf("Not enough minerals.\n");
            }
        } else if(c == 's') {
            pthread_mutex_lock(&commandCenter.commandCenter_mutex);
            if(commandCenter.minerals >= 50)
            {
                commandCenter.minerals -= 50;
                scv_all.scv_number += 1;
                scv_all.scv = realloc(scv_all.scv, sizeof(pthread_t) * scv_all.scv_number);
                // scv_all.scv_index = realloc(scv_all.scv_index, sizeof(int) * scv_all.scv_number);
                setupIndex(scv_all.scv_index, scv_all.scv_number);
                if(pthread_create(&scv_all.scv[scv_all.scv_number - 1], NULL, scv_mine, &scv_all.scv_index[scv_all.scv_number - 1]) != 0)
                {
                    perror("pthread_create is not successful");
                    free(mineralBlock);
                    free(scv_all.scv);
                    free(scv_all.scv_index);
                    return NULL;
                }
                pthread_mutex_unlock(&commandCenter.commandCenter_mutex);
                sleep(4);
                printf("SCV good to go, sir.\n");
            } else {
                pthread_mutex_unlock(&commandCenter.commandCenter_mutex);
                printf("Not enough minerals.\n");
            }
        }
        if(commandCenter.knights_number == 20)
        {
            return NULL;
        }
    }
}


int main(int argc, char *argv[])
{
    if(argc == 2)
    {
        commandCenter.mineralBlocks_number = atoi(argv[1]);
    } else {
        commandCenter.mineralBlocks_number = 2;
    }

    // Инициализиране минерални блокове
    mineralBlock = malloc(sizeof(struct MineralBlock) * commandCenter.mineralBlocks_number);
    for(int i = 0; i < commandCenter.mineralBlocks_number; i++)
    {
        mineralBlock[i].minerals = 500;
        pthread_mutex_init(&mineralBlock[i].mutex, NULL);
    }

    // Инициализиране работници
    scv_all.scv_number = 5;
    scv_all.scv = malloc(sizeof(pthread_t) * scv_all.scv_number);
    scv_all.scv_index = malloc(sizeof(int) * 180);
    setupIndex(scv_all.scv_index, scv_all.scv_number);
    // for(int i = 0; i < scv_all.scv_number; i++)
    // {
    //     printf("i: %d\n", scv_all.scv_index[i]);
    // }
    
    // Инициализиране команден център
    commandCenter.minerals = 0;
    commandCenter.knights_number = 0;
    pthread_mutex_init(&commandCenter.commandCenter_mutex, NULL);
    if(pthread_create(&commandCenter.commandCenter_thread, NULL, commandCenter_work, NULL) != 0)
    {
        perror("pthread_create is not successful");
        free(mineralBlock);
        free(scv_all.scv);
        free(scv_all.scv_index);
        exit(1);
    }

    for(int i = 0; i < scv_all.scv_number; i++)
    {
        if(pthread_create(&scv_all.scv[i], NULL, scv_mine, &scv_all.scv_index[i]) != 0)
        {
            perror("pthread_create is not successful");
            free(mineralBlock);
            free(scv_all.scv);
            free(scv_all.scv_index);
            exit(1);
        }
    }

    // join-ове
    if(pthread_join(commandCenter.commandCenter_thread, NULL) != 0)
    {
        perror("pthread_join is not successful");
        free(mineralBlock);
        free(scv_all.scv);
        free(scv_all.scv_index);
        exit(1);
    }

    for(int i = 0; i < scv_all.scv_number; i++)
    {
        if(pthread_join(scv_all.scv[i], NULL) != 0)
        {
            perror("pthread_join is not successful");
            free(mineralBlock);
            free(scv_all.scv);
            free(scv_all.scv_index);
            exit(1);
        }
    }

    // destroy-вания
    pthread_mutex_destroy(&commandCenter.commandCenter_mutex);
    for(int i = 0; i < commandCenter.mineralBlocks_number; i++)
    {
        pthread_mutex_destroy(&mineralBlock[i].mutex);
    }

    free(mineralBlock);
    free(scv_all.scv);
    free(scv_all.scv_index);

    printf("Map minerals %d, player minerals %d, SCVs %d, Marines %d\n", commandCenter.mineralBlocks_number * 500, commandCenter.minerals, scv_all.scv_number, commandCenter.knights_number);

    return 0;
}
