/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include <unistd.h>
#include <fcntl.h>

#include "defines.h"
#include "err_exit.h"


char *g_fifo1 = "/tmp/fifo1";
char *g_fifo2 = "/tmp/fifo2";
int g_msgKey = 1337;
int g_shmKey = 228;
int g_semKey = 23;

int open_fifo(char *path, int flag) {
    int fd = open(path, flag);
    if (fd == -1) {
        ErrExit("open failed");
    }

    return fd;
}

void write_fifo(int fd, message_struct *buffer, int size) {
    if (write(fd, buffer, size) != size) {
        ErrExit("write failed");
    }
    if (close(fd) != 0) {
        ErrExit("close failed");
    }
}

void read_from_file(int fd, char *buffer, int size) {
    if (read(fd, buffer, size) == -1) {
        ErrExit("read failed");
    }
    buffer[size] = '\0';
}
