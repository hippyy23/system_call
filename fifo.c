/// @file fifo.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione delle FIFO.

//#include <stdio.h>
//#include <unistd.h>
#include <fcntl.h>

//#include <sys/types.h>
#include <sys/stat.h>

#include "err_exit.h"
#include "fifo.h"


void make_fifo1(const char *path) {
    if (mkfifo(path, O_RDONLY) == -1) {
        ErrExit("mkfifo1 failed");
    }
}