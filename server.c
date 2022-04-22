/// @file sender_manager.c
/// @brief Contiene l'implementazione del sender_manager.

#include <string.h>

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"

int main(int argc, char * argv[]) {
    // initialize fifo1 pathname
    strcpy(fifo1, "/tmp/fifo1");
    // initialize fifo2 pathname
    strcpy(fifo2, "/tmp/fifo2");
    // make fifo1
    make_fifo1(fifo1);
    
    //make_fifo2(fifo2);

    return 0;
}
