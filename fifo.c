/// @file fifo.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione delle FIFO.

#include <stdio.h>
//#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "err_exit.h"
#include "fifo.h"


void make_fifo1(char *path) {
    printf("%s\n", path);
    if (mkfifo(path, S_IRUSR | S_IWUSR) == -1) {
        ErrExit("mkfifo1 failed");
    }
}
