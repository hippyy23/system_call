/// @file sender_manager.c
/// @brief Contiene l'implementazione del sender_manager.

//#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

//#include <sys/types.h>
//#include <sys/stat.h>

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"


int main(int argc, char * argv[]) {
    // make fifo1
    make_fifo1(fifo1);
    printf("<Server> FIFO1 %s created\n", fifo1);
    //make_fifo2(fifo2);

    if (signal(SIGINT, terminate_server) == SIG_ERR) {
        ErrExit("change signal handler failed");
    }

    printf("<Server> waiting for the number of files...\n");
    int fifo1FD = open(fifo1, O_RDONLY);
    if (fifo1FD == -1) {
        ErrExit("open failed");
    }

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
        // || close(fd2) != 0) {
        ErrExit("close failed");
    }

    if (unlink(fifo1) != 0) {
        // || unlink(fifo2) !=0) {
        ErrExit("unlink failed");
    }

    return 0;
}
