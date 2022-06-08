/// @file err_exit.c
/// @brief Contiene l'implementazione della funzione di stampa degli errori.

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

#include "err_exit.h"


/**
 * @brief Error exit
 * 
 * @param msg const char*
 */
void ErrExit(const char *msg)
{
    perror(msg);
    exit(1);
}
