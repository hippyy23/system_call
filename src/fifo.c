/// @file fifo.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione delle FIFO.

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "err_exit.h"
#include "fifo.h"
#include "defines.h"


/**
 * @brief Create a new FIFO named PATH,
 *        with persmissions READ and WRITE
 * 
 * @param path char*
 */
void make_fifo(char *path) {
    if (mkfifo(path, S_IRUSR | S_IWUSR) == -1) {
        ErrExit("mkfifo failed");
    }
}

/**
 * @brief Open FIFO
 * 
 * @param path char*
 * @param flag int
 * @return int 
 */
int open_fifo(char *path, int flg) {
    int fd = open(path, flg);
    if (fd == -1) {
        ErrExit("open failed");
    }

    return fd;
}

/**
 * @brief Read message from FIFO
 * 
 * @param fd int
 * @param m message_struct*
 */
void read_fifo(int fd, message_struct *dest) {
    if (read(fd, dest, sizeof(message_struct)) == -1) {
        ErrExit("read failed");
    }
}

/**
 * @brief Write message to FIFO
 * 
 * @param fifoFD int
 * @param m message_struct*
 */
void write_fifo(int fifoFD, message_struct *src) {
    if (write(fifoFD, src, sizeof(message_struct)) == -1) {
        ErrExit("write failed");
    }
}

/**
 * @brief Close and unlink FIFO
 * 
 * @param fifoFD int
 * @param fifoPath char*
 */
void close_fifo(int fifoFD, char *fifoPath) {
    errno = 0;
    if (close(fifoFD) != 0 && errno != EBADF) {
        ErrExit("close failed");  
    }
    if (unlink(fifoPath) != 0) {
        ErrExit("unlink failed");
    } else {
        printf("<Server> FIFO removed successfully\n");
    }
}
