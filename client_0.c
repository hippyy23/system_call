/// @file client.c
/// @brief Contiene l'implementazione del client.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

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
    strncpy(g_wd, argv[1], NAME_MAX);

    // allocate memory for MAX_FILES files
    alloc_mem_files();

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
    if (signal(SIGUSR1, quit) == SIG_ERR ||
        signal(SIGINT, start) == SIG_ERR)
        ErrExit("change signal handler failed");

    printf("Waiting for a signal...\n");
    pause();

    // search all files that start with "send_"
    search(0);
    // get the number of files found
    int numFiles = get_num_files();
    printf("<Client_0> Numero di file trovati %d\n", numFiles);

    // free allocated memory
    free_mem_files();

    return 0;
}