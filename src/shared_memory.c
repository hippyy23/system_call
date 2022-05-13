/// @file shared_memory.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione della MEMORIA CONDIVISA.

#include <stdio.h>

#include <sys/shm.h>
#include <sys/stat.h>

#include "err_exit.h"
#include "shared_memory.h"


/**
 * @brief Create/Get SHARED MEMORY SEGMENT
 * 
 * @param shmKey key_t 
 * @param size size_t
 * @return int: shmid 
 */
int alloc_shared_memory(key_t shmKey, size_t size) {
    int shmid = shmget(shmKey, size, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shmid == -1) {
        ErrExit("shmget failed");
    }

    return shmid;
}

/**
 * @brief Attach SHARED MEMORY SEGMENT
 * 
 * @param shmid int
 * @param shmflg int
 * @return void*
 */
void * attach_shared_memory(int shmid, int shmflg) {
    void *ptr = shmat(shmid, NULL, shmflg);
    if (ptr == (void *) -1) {
        ErrExit("shmat failed");
    }

    return ptr;
}

/**
 * @brief Detach SHARED MEMORY SEGMENT
 * 
 * @param ptr void*
 */
void free_shared_memory(void *ptr) {
    if (shmdt(ptr) == -1) {
        ErrExit("shmdt failed");
    }  else {
        printf("<Server> Shared Memory detached successfully\n");
    }
}

/**
 * @brief Read message from SHARED MEMORY SEGMENT
 * 
 * @param src message_struct*
 * @param dest message_struct*
 * @param shmArr short*
 */
int read_shdm(message_struct *src, message_struct *dest, short *shmArr) {
    for (int index = 0; index < MAX_MESSAGES_PER_IPC; index++) {
        if (shmArr[index] == 1) {
            *dest = src[index];
            shmArr[index] = 0;
            return 0;
        }
    }

    return -1;
}

/**
 * @brief Write message to SHARED MEMORY SEGMENT
 * 
 * @param src message_struct*
 * @param dest message_struct*
 * @param shmArr short*
 * @return int 
 */
int write_shdm(message_struct *src, message_struct *dest, short *shmArr) {
    // write the message on shared memory
    for (int index = 0; index < MAX_MESSAGES_PER_IPC; index++) {
        if (shmArr[index] == 0) {
            dest[index] = *src;
            shmArr[index] = 1;
            return 0;
        }
    }

    return -1;
}

/**
 * @brief Remove SHARED MEMORY SEGMENT
 * 
 * @param int shmid 
 */
void remove_shared_memory(int shmid) {
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        ErrExit("shmctl failed");
    } else {
        printf("<Server> Shared Memory removed successfully\n");
    }
}
