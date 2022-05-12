/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "defines.h"
#include "err_exit.h"


char *g_fifo1 = "/tmp/fifo1";
char *g_fifo2 = "/tmp/fifo2";
