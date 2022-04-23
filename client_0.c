/// @file client.c
/// @brief Contiene l'implementazione del client.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#include "defines.h"
#include "err_exit.h"


int main(int argc, char * argv[]) {
    // check command line input arguments
    if (argc != 2) {
        printf("Usage: %s path\n", argv[0]);
        return 0;
    }
    g_wd = argv[1];

    // create a 2d array to contain the path of MAX_FILES files
    char files[MAX_FILES][NAME_MAX];

    // create a mask to block all signals but SIGINT and SIGUSR1
    // set of signals
    sigset_t signalSet;
    // initialize signalSet
    sigfillset(&signalSet);
    // remove SIGINT and SIGUSR1 from the set
    sigdelset(&signalSet, SIGINT);
    sigdelset(&signalSet, SIGUSR1);
    // block all signals but SIGINT and SIGUSR1
    sigprocmask(SIG_SETMASK, &signalSet, NULL);

    // if SIGUSR1 is recieved client_0 finishes
    // if SIGINT is recieved client_0 starts
    if (signal(SIGUSR1, terminate_client0) == SIG_ERR ||
        signal(SIGINT, start_client0) == SIG_ERR)
        ErrExit("change signal handler failed");

    // wait for a signal to be recieved
    printf("Waiting for a signal...\n");
    pause();

    // search for all files whose name starts with "send_"
    search(files, 0);
    // get the number of files found
    int numFiles = get_num_files(files);
    printf("<Client_0> Numero di file trovati %d\n", numFiles);

    // send the number of files to the server through fifo1
    printf("<Client_0> opening fifo1 %s\n", fifo1);
    int fifo1FD = open(fifo1, O_WRONLY);
    if (fifo1FD == -1) {
        ErrExit("open failed");
    }

    if (write(fifo1FD, &numFiles, sizeof(numFiles)) != sizeof(numFiles)) {
        ErrExit("write failed");
    }

    if (close(fifo1FD) != 0) {
        ErrExit("close failed");
    }

    return 0;
}
