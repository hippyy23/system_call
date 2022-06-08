/// @file server.c
/// @brief Contiene l'implementazione del server.

#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <sys/msg.h>

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "message_queue.h"
#include "semaphore.h"
#include "fifo.h"
#include "server_functions.h"


void terminate_server() {
    remove_msgq(msqid);
    printf("<Server> Message Queue removed successfully\n");

    remove_sem(semid);
    printf("<Server> Semaphore Set removed successfully\n");

    free_shared_memory(shmPtr);
    // set shmArr pointer to its initial value before detaching
    shmArr -= (sizeof(message_struct) * MAX_MESSAGES_PER_IPC / 2);
    free_shared_memory(shmArr);
    printf("<Server> Shared Memory detached successfully\n");
    remove_shared_memory(shmid);
    printf("<Server> Shared Memory removed successfully\n");

    close_fifo(fifo1FD, g_fifo1);
    printf("<Server> FIFO1 removed successfully\n");
    close_fifo(fifo2FD, g_fifo2);
    printf("<Server> FIFO2 removed successfully\n");

    _exit(0);
}

int main(int argc, char * argv[]) {
    // make FIFO1 and FIFO2
    printf("<Server> creating FIFO1: %s...\n", g_fifo1);
    make_fifo(g_fifo1);
    printf("<Server> creating FIFO2: %s...\n", g_fifo2);
    make_fifo(g_fifo2);

    // create keys for IPCs
    key_t keyMsq = ftok(g_fifo1, 'a');
    key_t keyShm = ftok(g_fifo1, 'b');
    key_t keySem = ftok(g_fifo1, 'c');
    // check if keys have been created
    if (keyMsq == -1 || keyShm == -1 || keySem == -1) {
        ErrExit("ftok failed");
    }

    // create a MESSAGE QUEUE
    printf("<Server> creating MESSAGE QUEUE...\n");
    msqid = create_msgq(keyMsq, IPC_CREAT | S_IRUSR | S_IWUSR);

    // allocate a SHARED MEMORY SEGMENT
    printf("<Server> creating SHARED MEMORY SEGMENT...\n");
    shmid = alloc_shared_memory(keyShm, 
            sizeof(message_struct) * MAX_MESSAGES_PER_IPC + sizeof(short) * MAX_MESSAGES_PER_IPC);
    // attach the SHARED MEMORY SEGMENT in read/write mode
    shmPtr = (message_struct *) attach_shared_memory(shmid, 0);
    // attach an additional pointer to SHARED MEMORY SEGMENT
    // to handle the SHARED MEMORY SEGMENT allocation
    shmArr = (short *) attach_shared_memory(shmid, 0);
    // move the pointer to where the array of "short" starts
    shmArr = shmArr + (sizeof(message_struct) * MAX_MESSAGES_PER_IPC / 2);

    // create a semaphore set with 6 semaphore
    printf("<Server> creating SEMAPHORE SET...\n");
    semid = create_sem(keySem);

    // change signal handler for SIGINT
    if (signal(SIGINT, terminate_server) == SIG_ERR) {
        ErrExit("change signal handler failed");
    }

    // handle exit
    if (atexit(terminate_server) != 0) {
        _exit(EXIT_FAILURE);
    }

    while (1) {
        printf("<Server> waiting for the number of files...\n");
        // wait for unlock from client (sync purpose)
        semOp(semid, START, -1, 0);

        // open FIFO1 in read-only
        fifo1FD = open_fifo(g_fifo1, O_RDONLY);
        int numFiles;
        // read the number of files from FIFO1
        int bR = read(fifo1FD, &numFiles, sizeof(int));
        if (bR == -1) {
            printf("<Server> FIFO is broken\n");
        } else if (bR != sizeof(int) || bR == 0) {
            printf("<Server> did not recieved the number of files\n");
        } else {
            printf("<Server> number of files to be recieved %d\n", numFiles);
        }
        close(fifo1FD);

        // write init signal to client through shared memory
        printf("<Server> writing start signal to client\n");
        shmPtr[0].pid = -23;

        // allocate space for messages
        initialize_space_for_msg(numFiles);
        // open FIFO1 and FIFO2 in non-blocking mode
        fifo1FD = open_fifo(g_fifo1, O_RDONLY | O_NONBLOCK);
        fifo2FD = open_fifo(g_fifo2, O_RDONLY | O_NONBLOCK);

        // vars that indicate number of messages recieved (for each IPC) and also the index (for each container)
        int fifo1Rcv = 0;
        int fifo2Rcv = 0;
        int msqRcv = 0;
        int shmRcv = 0;

        // iterate until all messages have been received
        printf("<Server> recieving messages from client...\n");
        while (fifo1Rcv < numFiles || fifo2Rcv < numFiles || msqRcv < numFiles || shmRcv < numFiles) {
            // check whether all messages have already been received
            if (fifo1Rcv < numFiles) {
                message_struct m1;
                if (read_fifo(fifo1FD, &m1) == 0) {
                    semOp(semid, LIMIT_FIFO1, 1, 0);
                    container_fifo1[fifo1Rcv++] = m1;
                    // printf("[Parte 1, del file %s, spedita dal processo %d tramite FIFO1]\n%s\n", m1.path, m1.pid, m1.content);
                }                
            }

            // check whether all messages have already been received
            if (fifo2Rcv < numFiles) {
                message_struct m2;
                if (read_fifo(fifo2FD, &m2) == 0) {
                    semOp(semid, LIMIT_FIFO2, 1, 0);
                    container_fifo2[fifo2Rcv++] = m2;
                    // printf("[Parte 2, del file %s, spedita dal processo %d tramite FIFO2]\n%s\n", m2.path, m2.pid, m2.content);
                }
            }

            // check whether all messages have already been received
            if (msqRcv < numFiles) {
                msgqueue_struct m3;
                if (read_msgq(msqid, &m3, 1, IPC_NOWAIT) == 0) {
                    semOp(semid, LIMIT_MSGQ, 1, 0);
                    container_msgq[msqRcv++] = m3;
                    // printf("[Parte 3, del file %s, spedita dal processo %d tramite MsgQueue]\n%s\n", m3.mtext.path, m3.mtext.pid, m3.mtext.content);
                }
            }

            // check whether all messages have already been received
            if (shmRcv < numFiles) {
                message_struct m4;
                // printf("<Server> getting mutex on shared memory...\n");
                semOp(semid, MUTEX_SHM, -1, 0);
                // printf("<Server> reading shared memory...\n");
                if (read_shdm(shmPtr, &m4, shmArr) == 0) {
                    container_shdm[shmRcv++] = m4;
                    // printf("[Parte 4, del file %s, spedita dal processo %d tramite ShdMem]\n%s\n", m4.path, m4.pid, m4.content);
                }
                // printf("<Server> releasing mutex...\n");
                semOp(semid, MUTEX_SHM, 1, 0);
            }
        }
        printf("<Server> all messages were received\n");
        // close FIFO
        close(fifo1FD);
        close(fifo2FD);

        // write messages to output files
        printf("<Server> writing messages to files...\n");
        write_messages_to_files(numFiles);

        // free containers
        free(container_fifo1);
        free(container_fifo2);
        free(container_msgq);
        free(container_shdm);

        // send END message to client
        printf("<Server> sending end message to client\n\n");
        msgqueue_struct end;
        end.mtype = 1337;
        while (write_msgq(msqid, &end) != 0);
    }

    return 0;
}
