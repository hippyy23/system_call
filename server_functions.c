/// @file server_functions.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per il server.

#include "server_functions.h"
#include "defines.h"
#include "err_exit.h"


void initialize_space_for_msg(int numFiles) {
    container_fifo1 = malloc(sizeof(message_struct) * numFiles);
    container_fifo2 = malloc(sizeof(message_struct) * numFiles);
    container_msgq = malloc(sizeof(message_struct) * numFiles);
    container_shdm = malloc(sizeof(message_struct) * numFiles);

    if (container_fifo1 == NULL || container_fifo2 == NULL || 
    container_msgq == NULL || container_shdm == NULL) {
        ErrExit("malloca failed");
    }
}