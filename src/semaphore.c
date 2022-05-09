/// @file semaphore.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione dei SEMAFORI.

#include <stdio.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/sem.h>

#include "err_exit.h"
#include "semaphore.h"


/**
 * @brief Create a sem set with 7 semaphores
 * 
 * @param semKey int
 * @return int: semid 
 */
int create_sem(int key) {
    // create a semaphore set with 7 semaphore
    int semid = semget(key, 7, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (semid == -1) {
        ErrExit("semget failed");
    }

    return semid;
}

/**
 * @brief Initialize sem set
 * 
 * @param semid int
 * @param num_child int
 */
void initialize_sem(int semid, int num_child) {
    // initialize the semaphore set
    union semun arg;
    unsigned short values[] = {num_child, 1, 50, 50, 50, 0, 0};
    arg.array = values;

    if (semctl(semid, 0, SETALL, arg) == -1) {
        ErrExit("semctl SETALL failed");
    }
}

/**
 * @brief Remove sem set
 * 
 * @param semid int
 */
void remove_sem(int semid) {
    if (semctl(semid, 0, IPC_RMID, 0) == -1) {
        ErrExit("semctl IPC_RMID failed");
    } else {
        printf("<Server> Semaphore Set removed successfully\n");
    }
}

/**
 * @brief Operate on semaphore
 * 
 * @param semid int
 * @param sem_num unsigned short
 * @param sem_op short
 * @param sem_flg short
 * @return int 
 */
int semOp(int semid, unsigned short sem_num, short sem_op, short sem_flg) {
    struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = sem_flg};
    errno = 0;
    
    if (semop(semid, &sop, 1) == -1) {
        if (errno == EAGAIN) {
            return -1;
        } else {
            ErrExit("semop failed");
        }
    }
    
    return 0;
}