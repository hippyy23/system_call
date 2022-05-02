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

#include <sys/types.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/msg.h>

#include "defines.h"
#include "client_functions.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "err_exit.h"


int main(int argc, char * argv[]) {
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
        search(0);
        // get the number of files found
        int numFiles = get_num_files(g_files);
        printf("<Client_0> number of files found %d\n", numFiles);

        // send the number of files to the server through fifo1
        printf("<Client_0> opening fifo1 %s\n", g_fifo1);
        int fifo1FD = open_fifo(g_fifo1, O_WRONLY);

        if (write(fifo1FD, &numFiles, sizeof(numFiles)) != sizeof(numFiles)) {
            ErrExit("write failed");
        }

        int msqid = msgget(g_msgKey, 0);
        if (msqid == -1) {
            ErrExit("msgget failed");
        }

        printf("<Client_0> getting server's shared memory segment...\n");
        // get the server's shared memory segment
        int shmid = alloc_shared_memory(g_shmKey, sizeof(message_struct) * MAX_MESSAGES_PER_IPC);
        // attach the SHARED MEMORY SEGMENT in read/write mode
        message_struct *shmPtr = (message_struct *) attach_shared_memory(shmid, 0);
        // attach an additional pointer to SHARED MEMORY SEGMENT
        // to handle the SHARED MEMORY SEGMENT allocation
        short *shmArr = (short *) attach_shared_memory(shmid, 0);
        // move the pointer to where the array of "short" starts
        shmArr = shmArr + (sizeof(message_struct) * MAX_MESSAGES_PER_IPC / 2);

        // get the semaphore set
        printf("<Client_0> getting server's semaphore set...\n");
        int semid = semget(g_semKey, 5, 0);
        int fifo2FD = open_fifo(g_fifo2, O_WRONLY);
        if (semid != -1) {
            // wait for response from server on shared memory segment
            while (shmPtr[0].pid != -23);
            printf("<Client_0> recieved start signal from server\n");
            for (int child = 0; child < numFiles; child++) {
                pid_t pid = fork();
                message_struct m;
                msgqueue_struct mq;

                if (pid == -1) {
                    ErrExit("fork failed");
                // check whether the running process is the parent or the child
                } else if (pid == 0) {
                    // code executed only by the child
                    pid_t pid = getpid();
                    // OPEN FILE
                    printf("<Client_%d> opening file %s\n", pid, g_files[child]);
                    int fileFD = open(g_files[child], O_RDONLY, S_IRUSR);
                    if (fileFD == -1) {
                        ErrExit("open failed");
                    }

                    // CHECK THE NUMBER OF CHARS
                    int numChars = check_num_chars_in_file(fileFD);
                    if (numChars < 4) {
                        printf("ERROR: number of chars less then 4");
                    }
                    printf("<Client_%d> number of chars found %d\n", pid, numChars);

                    // GET THE SIZE OF EACH PART
                    // we have problems with len < 10 (5, 6, 9)
                    // if 5 - 2 1 1 1
                    // if 6 - 3 1 1 1
                    // if 9 - 3 2 2 2
                    // for the rest use ceil(numChars / 4)
                    int size = ceil(((float) numChars) / 4);

                    // BLOCK ALL CHILDS UNTIL EVERYONE HAS REACHED THIS POINT
                    semOp(semid, WAIT_CHILD, -1);
                    semOp(semid, WAIT_CHILD, 0);

                    // SEND EACH PART THROUGH IPCS
                    switch (numChars) {
                        case 5:
                            // 1. FIFO1
                            // write on FIFO1 the 1st part of the file of the size 2
                            printf("<Client_%d> writing on FIFO1...\n", pid);
                            m = create_message_struct(fileFD, pid, child, 2);
                            write_fifo(fifo1FD, &m);
                            semOp(semid, SYNC_FIFO1, 1);

                            // 2. FIFO2
                            // write on FIFO2 the 2nd part of the file of the size 1
                            printf("<Client_%d> writing on FIFO2...\n", pid);
                            m = create_message_struct(fileFD, pid, child, 1);
                            write_fifo(fifo2FD, &m);
                            semOp(semid, SYNC_FIFO2, 1);

                            // 3. MSGQUEUE
                            // send through MsgQueue the 2nd part of the file of the size 1
                            printf("<Client_%d> sending message through msgqueue...\n", pid);
                            mq = create_msgqueue_struct(fileFD, pid, child, 1);
                            write_msgq(msqid, &mq);

                            // 4. SHDMEM
                            // write on shared memory the 4th part of the file of the size 1
                            m = create_message_struct(fileFD, pid, child, 1);
                            // get mutex on shared memory
                            printf("<Client_%d> getting mutex on shared memory...\n", pid);
                            semOp(semid, MUTEX_SHM, -1);
                            printf("<Client_%d> writing on shared memory...\n", pid);
                            write_shdm(&m, shmPtr, shmArr);
                            // realease mutex
                            printf("<Client_%d> releasing mutex...\n", pid);
                            semOp(semid, SYNC_SHM, 1);
                            semOp(semid, MUTEX_SHM, 1);
                            break;
                        case 6:
                            // 1. FIFO1
                            // write on FIFO1 the 1st part of the file of the size 3'
                            printf("<Client_%d> writing on FIFO1...\n", pid);
                            m = create_message_struct(fileFD, pid, child, 3);
                            write_fifo(fifo1FD, &m);
                            semOp(semid, SYNC_FIFO1, 1);

                            // 2. FIFO2
                            // write on FIFO2 the 2nd part of the file of the size 1
                            printf("<Client_%d> writing on FIFO2...\n", pid);
                            m = create_message_struct(fileFD, pid, child, 1);
                            write_fifo(fifo2FD, &m);
                            semOp(semid, SYNC_FIFO2, 1);

                            // 3. MSGQUEUE
                            // send through MsgQueue the 3rd part of the file of the size 1
                            printf("<Client_%d> sending message through msgqueue...\n", pid);
                            mq = create_msgqueue_struct(fileFD, pid, child, 1);
                            write_msgq(msqid, &mq);

                            // 4. SHDMEM
                            // write on shared memory the 4th part of the file of the size 1
                            m = create_message_struct(fileFD, pid, child, 1);
                            // get mutex on shared memory
                            printf("<Client_%d> getting mutex on shared memory...\n", pid);
                            semOp(semid, MUTEX_SHM, -1);
                            printf("<Client_%d> writing on shared memory...\n", pid);
                            write_shdm(&m, shmPtr, shmArr);
                            // realease mutex
                            printf("<Client_%d> releasing mutex...\n", pid);
                            semOp(semid, SYNC_SHM, 1);
                            semOp(semid, MUTEX_SHM, 1);
                            break;
                        case 9:
                            // 1. FIFO1
                            // write on FIFO1 the 1st part of the file of the size 3
                            printf("<Client_%d> writing on FIFO1...\n", pid);
                            m = create_message_struct(fileFD, pid, child, 3);
                            write_fifo(fifo1FD, &m);
                            semOp(semid, SYNC_FIFO1, 1);

                            // 2. FIFO2
                            // write on FIFO2 the 2nd part of the file of the size 2
                            printf("<Client_%d> writing on FIFO2...\n", pid);
                            m = create_message_struct(fileFD, pid, child, 2);
                            write_fifo(fifo2FD, &m);
                            semOp(semid, SYNC_FIFO2, 1);

                            // 3. MSGQUEUE
                            // send through MsgQueue the 3rd part of the file of the size 2
                            printf("<Client_%d> sending message through msgqueue...\n", pid);
                            mq = create_msgqueue_struct(fileFD, pid, child, 2);
                            write_msgq(msqid, &mq);

                            // 4. SHDMEM
                            // write on shared memory the 4th part of the file of the size 2
                            m = create_message_struct(fileFD, pid, child, 2);
                           // get mutex on shared memory
                            printf("<Client_%d> getting mutex on shared memory...\n", pid);
                            semOp(semid, MUTEX_SHM, -1);
                            printf("<Client_%d> writing on shared memory...\n", pid);
                            write_shdm(&m, shmPtr, shmArr);
                            // realease mutex
                            printf("<Client_%d> releasing mutex...\n", pid);
                            semOp(semid, SYNC_SHM, 1);
                            semOp(semid, MUTEX_SHM, 1);
                            break;
                        default:
                            // 1. FIFO1
                            // write on FIFO1 the 1st part of the file of the size 'size'
                            printf("<Client_%d> writing on FIFO1...\n", pid);
                            m = create_message_struct(fileFD, pid, child, size);
                            write_fifo(fifo1FD, &m);               
                            semOp(semid, SYNC_FIFO1, 1);

                            // 2. FIFO2
                            // write on FIFO2 the 2nd part of the file of the size 'size'
                            printf("<Client_%d> writing on FIFO2...\n", pid);
                            m = create_message_struct(fileFD, pid, child, size);
                            write_fifo(fifo2FD, &m);          
                            semOp(semid, SYNC_FIFO2, 1);

                            // 3. MSGQUEUE
                            // send through MsgQueue the 3rd part of the file of the size 'size'
                            printf("<Client_%d> sending message through msgqueue...\n", pid);
                            mq = create_msgqueue_struct(fileFD, pid, child, size);
                            write_msgq(msqid, &mq);

                            // 4. SHDMEM
                            // write on shared memory the 4th part of the file of the size 'size'
                            m = create_message_struct(fileFD, pid, child, size);
                            // get mutex on shared memory
                            printf("<Client_%d> getting mutex on shared memory...\n", pid);
                            semOp(semid, MUTEX_SHM, -1);
                            printf("<Client_%d> writing on shared memory...\n", pid);
                            write_shdm(&m, shmPtr, shmArr);
                            // realease mutex
                            printf("<Client_%d> releasing mutex...\n", pid);
                            semOp(semid, SYNC_SHM, 1);
                            semOp(semid, MUTEX_SHM, 1);
                            break;
                    }

                    // close file
                    printf("<Client_%d> closing the file %s...\n", pid, g_files[child]);
                    close(fileFD);
                    close(fifo1FD);
                    close(fifo2FD);
                    exit(0);
                }
            }
            // code executed only by the parent
            // wait the termination of all child processes
            while(wait(NULL) != -1);

            printf("<Client_0> waiting for end message from the server...\n");
            msgqueue_struct end;
            size_t mSize = sizeof(msgqueue_struct) - sizeof(long);
            if (msgrcv(msqid, &end, mSize, 1337, 0) == -1) {
                ErrExit("msgrcv failed");
            }
            printf("<Client_0> recieved end message from the server\n\n");

            unlock_signals();

        } else {
            ErrExit("semget failed");
        }
    }
    printf("<Client_0> quitting...\n");
    return 0;
}
