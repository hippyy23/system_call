/// @file sender_manager.c
/// @brief Contiene l'implementazione del sender_manager.

//#include <stdio.h>
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
int fifo1FD;
int fifo2FD;

void terminate_server() {
    printf("Ctrl+c fuck off!");
    free_shared_memory(shmPtr);
    remove_shared_memory(shmid);
    if (close(fifo1FD) != 0) {
        // || close(fd2) != 0) {
        ErrExit("close failed");
    }

    if (unlink(g_fifo1) != 0) {
        // || unlink(g_fifo2) !=0) {
        ErrExit("unlink failed");
    }
    _exit(0);
}

int main(int argc, char * argv[]) {
    // make FIFO1 and FIFO2
    printf("<Server> creating FIFO1 %s\n", g_fifo1);
    make_fifo(g_fifo1);
    // make_fifo(g_fifo2);
    // printf("<Server> FIFO1 %s created\n", g_fifo2);

    // create a MESSAGE QUEUE
    int msqid = msgget(g_msgKey, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (msqid == -1) {
        ErrExit("msgget failed");
    }

    printf("<Server> creating shared memory segment\n");
    // allocate a SHARED MEMORY SEGMENT
    shmid = alloc_shared_memory(g_shmKey, SHDMEM_SIZE);
    // attach the SHARED MEMORY SEGMENT in read/write mode
    shmPtr = (char *) attach_shared_memory(shmid, 0);

    // create a semaphore set with 1 semapaphore
    printf("<Server> creating semaphore set\n");
    int semid = create_sem(g_semKey);

    // change signal handler for SIGINT
    if (signal(SIGINT, terminate_server) == SIG_ERR) {
        ErrExit("change signal handler failed");
    }

    // open FIFO1 in read-only
    printf("<Server> waiting for the number of files...\n");
    fifo1FD = open(g_fifo1, O_RDONLY);
    if (fifo1FD == -1) {
        ErrExit("open failed");
    }

    // read the number of files from FIFO1
    int numFiles;
    int bR = read(fifo1FD, &numFiles, sizeof(numFiles));
    if (bR == -1) {
        printf("<Server> FIFO is broken\n");
    } else if (bR != sizeof(numFiles) || bR == 0) {
        printf("<Server> did not recieved the number of files\n");
    } else {
        printf("<Server> number of files to be recieved %d\n", numFiles);
    }

    // write init signal '*' to client through shared memory
    printf("<Server> writing '*' to client\n");
    *shmPtr = '*';
    semOp(semid, 0, 1);

    if (close(fifo1FD) != 0) {
        // || close(fd2) != 0) {
        ErrExit("close failed");
    }

    if (unlink(g_fifo1) != 0) {
        // || unlink(g_fifo2) !=0) {
        ErrExit("unlink failed");
    }

    return 0;
}
