/// @file shared_memory.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione della MEMORIA CONDIVISA.

#include <stdio.h>

#include <sys/shm.h>
#include <sys/stat.h>

#include "err_exit.h"
#include "shared_memory.h"


// get/create a shared memory segment
int alloc_shared_memory(key_t shmKey, size_t size) {
    int shmid = shmget(shmKey, size, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shmid == -1) {
        ErrExit("shmget failed");
    }

    return shmid;
}

// attach the shared memory
void * attach_shared_memory(int shmid, int shmflg) {
    void *ptr = shmat(shmid, NULL, shmflg);
    if (ptr == (void *) -1) {
        ErrExit("shmat failed");
    }

    return ptr;
}

// detach the shared memory segment
void free_shared_memory(void *ptr) {
    if (shmdt(ptr) == -1) {
        ErrExit("shmdt failed");
    }  else {
        printf("<Server> Shared Memory detached successfully\n");
    }
}

// delete the shared memory segment
void remove_shared_memory(int shmid) {
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        ErrExit("shmctl failed");
    } else {
        printf("<Server> Shared Memory removed successfully\n");
    }
}
