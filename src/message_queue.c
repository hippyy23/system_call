/// @file message_queue.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione della MESSAGE QUEUE.

#include <stdlib.h>
#include <errno.h>

#include <sys/msg.h>

#include "message_queue.h"
#include "defines.h"
#include "err_exit.h"


/**
 * @brief Create/Get MESSAGE QUEUE
 * 
 * @param key int
 * @param flags int
 * @return int: msqid
 */
int create_msgq(int key, int flags) {
    int msqid = msgget(key, flags);
    if (msqid == -1) {
        ErrExit("msgget failed");
    }

    return msqid;
}

/**
 * @brief Recieve message from MESSAGE QUEUE
 * 
 * @param msqid int
 * @param dest msgqueue_struct*
 * @param mtype int
 * @param flg int
 * @return int
 */
int read_msgq(int msqid, msgqueue_struct *dest, int mtype, int flg) {
    size_t mSize = sizeof(msgqueue_struct) - sizeof(long);
    errno = 0;
    if (msgrcv(msqid, dest, mSize, mtype, flg) == -1) {
        if (errno == ENOMSG) {
            return -1;
        } else {
            ErrExit("msgrcv failed");
        }
    }

    return 0;
}

/**
 * @brief Send message to MESSAGE QUEUE
 * 
 * @param msqid int
 * @param m int
 * @return int 
 */
int write_msgq(int msqid, msgqueue_struct *src) {
    size_t mSize = sizeof(msgqueue_struct) - sizeof(long);
    errno = 0;
    if (msgsnd(msqid, src, mSize, IPC_NOWAIT) == -1) {
        if (errno == EAGAIN) {
            return -1;
        } else {
            ErrExit("msgsnd failed");
        }
    }

    return 0;
}

/**
 * @brief Remove MESSAGE QUEUE
 * 
 * @param msqid int
 */
void remove_msgq(int msqid) {
    if (msqid > 0) {
        if (msgctl(msqid, IPC_RMID, NULL) == -1) {
            ErrExit("msgctl failed");
        }
    }
}
