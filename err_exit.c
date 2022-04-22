/// @file err_exit.c
/// @brief Contiene l'implementazione della funzione di stampa degli errori.

#include "err_exit.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

void ErrExit(const char *msg)
{
    perror(msg);
    exit(1);
}
