/// @file semaphore.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione dei SEMAFORI.

#include <stdio.h>

#include <sys/stat.h>
#include <sys/sem.h>

#include "err_exit.h"
#include "semaphore.h"


int create_sem(key_t semKey) {
    // create a semaphore set with 1 semaphore
    int semid = semget(semKey, 1, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (semid == -1) {
        ErrExit("semget failed");
    }

    // initialize the semaphore set
    union semun arg;
    unsigned short values[] = {0};
    arg.array = values;

    if (semctl(semid, 0, SETALL, arg) == -1) {
        ErrExit("semctl SETALL failed");
    }

    return semid;
}

void semOp(int semid, unsigned short sem_num, short sem_op) {
    struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = 0};

    if (semop(semid, &sop, 1) == -1) {
        ErrExit("semop failed");
    }
}