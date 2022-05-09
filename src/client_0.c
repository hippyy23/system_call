/// @file client.c
/// @brief Contiene l'implementazione del client.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <limits.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/msg.h>

#include "defines.h"
#include "fifo.h"
#include "client_functions.h"
#include "message_queue.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "err_exit.h"


int main(int argc, char * argv[]) {
    int numFiles;
    int fifo1FD, fifo2FD;
    int semid, shmid, msqid;
    short *shmArr;

    // check command line input arguments
    if (argc != 2) {
        printf("Usage: %s path\n", argv[0]);
        return 0;
    }
    g_wd = argv[1];

    // create a mask to block all signals but SIGINT and SIGUSR1
    // initialize signalSet
    sigfillset(&signalSet);
    // remove SIGINT and SIGUSR1 from the set
    sigdelset(&signalSet, SIGINT);
    sigdelset(&signalSet, SIGUSR1);
    // block all signals but SIGINT and SIGUSR1
    sigprocmask(SIG_BLOCK, &signalSet, NULL);

    // if SIGUSR1 is recieved client_0 finishes
    // if SIGINT is recieved client_0 starts
    if (signal(SIGUSR1, terminate_client0) == SIG_ERR ||
        signal(SIGINT, start_client0) == SIG_ERR)
        ErrExit("change signal handler failed");

    while (1) {
        // wait for a signal to be recieved
        printf("waiting for a signal...\n");
        pause();
        block_all_signals();

        // search for all files whose name starts with "send_"
        search();
        // get the number of files found
        numFiles = get_num_files(g_files);
        printf("<Client_0> number of files found %d\n", numFiles);

        // send the number of files to the server through fifo1
        printf("<Client_0> opening fifo1 %s\n", g_fifo1);
        fifo1FD = open_fifo(g_fifo1, O_WRONLY);

        if (write(fifo1FD, &numFiles, sizeof(int)) != sizeof(int)) {
            ErrExit("write failed");
        }

        // get the server msqid
        msqid = create_msgq(g_msgKey, 0);

        printf("<Client_0> getting server's shared memory segment...\n");
        // get the server's shared memory segment
        shmid = alloc_shared_memory(g_shmKey, 
                    sizeof(message_struct) * MAX_MESSAGES_PER_IPC + sizeof(short) * MAX_MESSAGES_PER_IPC);
        // attach the SHARED MEMORY SEGMENT in read/write mode
        message_struct *shmPtr = (message_struct *) attach_shared_memory(shmid, 0);
        // attach an additional pointer to SHARED MEMORY SEGMENT
        // to handle the SHARED MEMORY SEGMENT allocation
        shmArr = (short *) attach_shared_memory(shmid, 0);
        // move the pointer to where the array of "short" starts
        shmArr = shmArr + (sizeof(message_struct) * MAX_MESSAGES_PER_IPC / 2);

        // get the semaphore set
        printf("<Client_0> getting server's semaphore set...\n");
        semid = semget(g_semKey, 7, 0);
        fifo2FD = open_fifo(g_fifo2, O_WRONLY);
        if (semid != -1) {
            // wait for response from server on shared memory segment
            while (shmPtr[0].pid != -23);

            printf("<Client_0> recieved start signal from server\n");
            for (int child = 0; child < numFiles; child++) {
                pid_t pid = fork();
                message_struct mFifo1;
                mFifo1.pid = 0;
                message_struct mFifo2;
                mFifo2.pid = 0;
                msgqueue_struct mMsgq;
                mMsgq.mtext.pid = 0;
                message_struct mShm;
                mShm.pid = 0;
                int sizeFifo1, sizeFifo2, sizeMsgq, sizeShm;
                
                if (pid == -1) {
                    ErrExit("fork failed");
                // check whether the running process is the parent or the child
                } else if (pid == 0) {
                    // code executed only by the child
                    pid_t pid = getpid();
                    // OPEN FILE
                    // printf("<Client_%d> opening file %s\n", pid, g_files[child]);
                    int fileFD = open(g_files[child], O_RDONLY, S_IRUSR);
                    if (fileFD == -1) {
                        ErrExit("open failed");
                    }

                    // CHECK THE NUMBER OF CHARS
                    int numChars = check_num_chars_in_file(fileFD);
                    if (numChars < 4) {
                        printf("ERROR: number of chars less then 4");
                    }
                    // printf("<Client_%d> number of chars found %d\n", pid, numChars);

                    // GET THE SIZE OF EACH PART
                    // we have problems with numChars less then 10 -> (5, 6, 9)
                    // if 5 - FIFO1: 2 FIFO2: 1 Msgq: 1 Shm: 1
                    // if 6 - FIFO1: 3 FIFO2: 1 Msgq: 1 Shm: 1
                    // if 9 - FIFO1: 3 FIFO2: 2 Msgq: 2 Shm: 2
                    // for the rest use ceil(numChars / 4)
                    int size = ceil(((float) numChars) / 4);
                    switch (numChars) {
                        case 5:
                            sizeFifo1 = 2;
                            sizeFifo2 = 1;
                            sizeMsgq = 1;
                            sizeShm = 1;
                            break;
                        case 6:
                            sizeFifo1 = 3;
                            sizeFifo2 = 1;
                            sizeMsgq = 1;
                            sizeShm = 1;
                            break;
                        case 9:
                            sizeFifo1 = 3;
                            sizeFifo2 = 2;
                            sizeMsgq = 2;
                            sizeShm = 2;
                            break;
                        default:
                            sizeFifo1 = size;
                            sizeFifo2 = size;
                            sizeMsgq = size;
                            sizeShm = size;
                            break;
                    }
                    // BLOCK ALL CHILDREN UNTIL EVERYONE HAS REACHED THIS POINT
                    semOp(semid, WAIT_CHILD, -1, 0);
                    semOp(semid, WAIT_CHILD, 0, 0);

                    // SEND EACH PART THROUGH IPCS
                    bool sentFifo1 = false;
                    bool sentFifo2 = false;
                    bool sentMsgq = false;
                    bool sentShm = false;
                    // flag to check if the function semOp failed with errno equal to EAGAIN
                    int res = 0;
                    do {
                        // 1. FIFO1
                        // write on FIFO1 the 1st part of the file
                        // check if the structure is initialized
                        if (mFifo1.pid == 0) {
                            // printf("<Client_%d> creating message struct fifo1...\n", pid);
                            mFifo1 = create_message_struct(fileFD, pid, child, sizeFifo1);
                        }
                        // check whether the message has been sent
                        if (!sentFifo1) {
                            res = semOp(semid, LIMIT_FIFO1, -1, IPC_NOWAIT);
                            if (res == 0) {
                                // printf("<Client_%d> writing on FIFO1...\n", pid);
                                write_fifo(fifo1FD, &mFifo1);
                                sentFifo1 = true;
                            }
                        }

                        // 2. FIFO2
                        // write on FIFO2 the 2nd part of the file
                        // check if the structure is initialized
                        if (mFifo2.pid == 0) {
                            // printf("<Client_%d> creating message struct fifo2...\n", pid);
                            mFifo2 = create_message_struct(fileFD, pid, child, sizeFifo2);
                        }
                        // check whether the message has been sent
                        if (!sentFifo2) {
                            res = semOp(semid, LIMIT_FIFO2, -1, IPC_NOWAIT);
                            if (res == 0) {
                                // printf("<Client_%d> writing on FIFO2...\n", pid);
                                write_fifo(fifo2FD, &mFifo2);
                                sentFifo2 = true;
                            }
                        }

                        // 3. MSGQUEUE
                        // send through MsgQueue the 3rd part of the file
                        // check if the structure is initialized
                        if (mMsgq.mtext.pid == 0) {
                            // printf("<Client_%d> creating msgq struct...\n", pid);
                            mMsgq = create_msgqueue_struct(fileFD, pid, child, sizeMsgq);
                        }
                        // check whether the message has been sent
                        if (!sentMsgq) {
                            res = semOp(semid, LIMIT_MSGQ, -1, IPC_NOWAIT);
                            if (res == 0) {
                                // printf("<Client_%d> sending message through msgqueue...\n", pid);
                                if (write_msgq(msqid, &mMsgq) == 0) {
                                    sentMsgq = true;
                                }
                            }
                        }

                        // 4. SHDMEM
                        // write on shared memory the 4th part of the file
                        // check if the structure is initialized
                        if (mShm.pid == 0) {
                            // printf("<Client_%d> creating message struct shm...\n", pid);
                            mShm = create_message_struct(fileFD, pid, child, sizeShm);
                        }
                        // check whether the message has been sent
                        if (!sentShm) {
                            // get mutex on shared memory
                            // printf("<Client_%d> getting mutex on shared memory...\n", pid);
                            semOp(semid, MUTEX_SHM, -1, 0);
                            // printf("<Client_%d> writing on shared memory...\n", pid);
                            if (write_shdm(&mShm, shmPtr, shmArr) == 0) {
                                sentShm = true;
                                semOp(semid, SYNC_SHM, 1, 0);
                            }
                            // realease mutex
                            // printf("<Client_%d> releasing mutex...\n", pid);
                            semOp(semid, MUTEX_SHM, 1, 0);
                        }
                    // iterate until all messages have been sent
                    } while (!sentFifo1 || !sentFifo2 || !sentMsgq || !sentShm);

                    // close file
                    // printf("<Client_%d> closing the file %s...\n", pid, g_files[child]);
                    close(fileFD);
                    exit(0);
                }
            }
            // code executed only by the parent
            // wait the termination of all child processes
            while(wait(NULL) != -1);
            // close fifo
            close(fifo1FD);
            close(fifo2FD);

            printf("<Client_0> waiting for end message from the server...\n");
            msgqueue_struct end;
            size_t mSize = sizeof(msgqueue_struct) - sizeof(long);
            // wait for the END message from server
            if (msgrcv(msqid, &end, mSize, 1337, 0) == -1) {
                ErrExit("msgrcv failed");
            }
            printf("<Client_0> recieved end message from the server\n\n");
            semOp(semid, END, 1, 0);

            unlock_signals();

        } else {
            ErrExit("semget failed");
        }
    }
    printf("<Client_0> quitting...\n");
    return 0;
}
