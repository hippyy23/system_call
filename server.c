/// @file sender_manager.c
/// @brief Contiene l'implementazione del sender_manager.

#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

//#include <sys/types.h>
//#include <sys/stat.h>
#include <sys/msg.h>

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"
#include "server_functions.h"


message_struct *shmPtr;
int shmid;
int msqid;
int semid;
int fifo1FD;
int fifo2FD;

void terminate_server() {
    remove_msgq(msqid);

    remove_sem(semid);

    free_shared_memory(shmPtr);
    remove_shared_memory(shmid);

    close_fifo(fifo1FD, g_fifo1);
    close_fifo(fifo2FD, g_fifo2);

    free(container_fifo1);
    free(container_fifo2);
    free(container_msgq);
    free(container_shdm);

    exit(0);
}

int main(int argc, char * argv[]) {
    // make FIFO1 and FIFO2
    printf("<Server> creating FIFO1 %s...\n", g_fifo1);
    make_fifo(g_fifo1);
    printf("<Server> creating FIFO2 %s...\n", g_fifo2);
    make_fifo(g_fifo2);

    // create a MESSAGE QUEUE
    msqid = msgget(g_msgKey, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (msqid == -1) {
        ErrExit("msgget failed");
    }

    printf("<Server> creating shared memory segment...\n");
    // allocate a SHARED MEMORY SEGMENT
    shmid = alloc_shared_memory(g_shmKey, sizeof(message_struct) * MAX_MESSAGES_PER_IPC);
    // attach the SHARED MEMORY SEGMENT in read/write mode
    shmPtr = (message_struct *) attach_shared_memory(shmid, 0);

    // create a semaphore set with 5 semapaphore
    printf("<Server> creating semaphore set...\n");
    semid = create_sem(g_semKey);

    // change signal handler for SIGINT
    if (signal(SIGINT, terminate_server) == SIG_ERR) {
        ErrExit("change signal handler failed");
    }

    while (1) {
        // open FIFO1 in read-only
        printf("<Server> waiting for the number of files...\n");

        // read the number of files from FIFO1
        //sleep(3);
        fifo1FD = open_fifo(g_fifo1, O_RDONLY);
        int numFiles;
        int bR = read(fifo1FD, &numFiles, sizeof(numFiles));
        if (bR == -1) {
            printf("<Server> FIFO is broken\n");
        } else if (bR != sizeof(numFiles) || bR == 0) {
            printf("<Server> did not recieved the number of files\n");
        } else {
            printf("<Server> number of files to be recieved %d\n", numFiles);
        }

        // initialize the semaphore
        initialize_sem(semid, numFiles);

        // write init signal '*' to client through shared memory
        printf("<Server> writing start signal to client\n");
        shmPtr[0].pid = -23;

        initialize_space_for_msg(numFiles);
        
        fifo2FD = open_fifo(g_fifo2, O_RDONLY);
        for (int i = 0; i < numFiles; i++) {
            semOp(semid, SYNC_FIFO1, -1);
            message_struct m1;
            read_fifo(fifo1FD, &m1, sizeof(m1));
            container_fifo1[i] = m1;

            printf("[Parte 1, del file %s, spedita dal processo %d tramite FIFO1]\n%s\n", m1.path, m1.pid, m1.content);

            message_struct m2;
            semOp(semid, SYNC_FIFO2, -1);
            read_fifo(fifo2FD, &m2, sizeof(m2));
            container_fifo2[i] = m2;

            printf("[Parte 2, del file %s, spedita dal processo %d tramite FIFO2]\n%s\n", m2.path, m2.pid, m2.content);

            msgqueue_struct m3;
            size_t mSize = sizeof(msgqueue_struct) - sizeof(long);
            if (msgrcv(msqid, &m3, mSize, 1, 0) == -1) {
                ErrExit("msgrcv failed");
            }
            container_msgq[i] = m3;

            printf("[Parte 3, del file %s, spedita dal processo %d tramite MsgQueue]\n%s\n", m3.mtext.path, m3.mtext.pid, m3.mtext.content);

            message_struct m4;
            // // read_shdm(&m4, shmPtr);
            // m4 = shmPtr[0];
            container_shdm[i] = m4;

            // printf("[Parte 4, del file %s, spedita dal processo %d tramite ShdMem]\n%s\n", m4.path, m4.pid, m4.content);
        }

        // write_messages_to_files(numFiles);

        printf("<Server> sending end message to client\n\n");
        msgqueue_struct end;
        end.mtype = 1337;
        size_t mSize = sizeof(msgqueue_struct) - sizeof(long);
        if (msgsnd(msqid, &end, mSize, 0) == -1) {
            ErrExit("msgsnd failed");
        }
    }

    return 0;
}
