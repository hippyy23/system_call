/// @file semaphore.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione dei SEMAFORI.

#include <stdio.h>

#include <sys/stat.h>
#include <sys/sem.h>

#include "err_exit.h"
#include "semaphore.h"


int create_sem(key_t semKey) {
    // create a semaphore set with 5 semaphore
    int semid = semget(semKey, 5, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (semid == -1) {
        ErrExit("semget failed");
    }

    return semid;
}

void initialize_sem(int semid, int num_child) {
    // initialize the semaphore set
    union semun arg;
    unsigned short values[] = {0, num_child, 1, 0, 0};
    arg.array = values;

    if (semctl(semid, 0, SETALL, arg) == -1) {
        ErrExit("semctl SETALL failed");
    }
}

void remove_sem(int semid) {
    if (semctl(semid, 0, IPC_RMID, 0) == -1) {
        ErrExit("semctl IPC_RMID failed");
    } else {
        printf("<Server> Semaphore Set removed successfully\n");
    }
}

void semOp(int semid, unsigned short sem_num, short sem_op) {
    struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = 0};

    if (semop(semid, &sop, 1) == -1) {
        ErrExit("semop failed");
    }
}