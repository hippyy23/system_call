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


char *shmPtr;
int shmid;
int msqid;
int semid;
int fifo1FD;
int fifo2FD;

void terminate_server() {
    if (msqid > 0) {
        if (msgctl(msqid, IPC_RMID, NULL) == -1)
            ErrExit("msgctl failed");
        else
            printf("<Server> message queue removed successfully\n");
    }
    remove_sem(semid);
    free_shared_memory(shmPtr);
    remove_shared_memory(shmid);

    close_fifo(fifo1FD, g_fifo1);
    close_fifo(fifo2FD, g_fifo2);
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
    shmPtr = (char *) attach_shared_memory(shmid, 0);

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
        if (close(fifo1FD) != 0) {
            ErrExit("close failed");
        }

        // initialize the semaphore
        initialize_sem(semid, numFiles);

        // write init signal '*' to client through shared memory
        printf("<Server> writing '*' to client\n");
        *shmPtr = '*';
        semOp(semid, START_END, 1);

        // capire come far si che il server legga ciclicamente dalle ipcs
        while (1) {
            fifo1FD = open_fifo(g_fifo1, O_RDONLY);
            message_struct m1;
            bR = read(fifo1FD, &m1, sizeof(m1));
            if (bR == -1) {
                printf("<Server> FIFO is broken\n");
            }
            if (close(fifo1FD) != 0) {
                ErrExit("close failed");
            }

            printf("[Parte 1, del file %s, spedita dal processo %d tramite FIFO1]\n%s\n", m1.path, m1.pid, m1.content);

            fifo2FD = open_fifo(g_fifo2, O_RDONLY);
            message_struct m2;
            bR = read(fifo2FD, &m2, sizeof(m2));
            if (bR == -1) {
                printf("<Server> FIFO is broken\n");
            }
            if (close(fifo2FD) != 0) {
                ErrExit("close failed");
            }

            printf("[Parte 2, del file %s, spedita dal processo %d tramite FIFO2]\n%s\n", m2.path, m2.pid, m2.content);
        }

    }

    return 0;
}
