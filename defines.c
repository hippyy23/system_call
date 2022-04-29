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
int g_msgKey = 1337;
int g_shmKey = 228;
int g_semKey = 23;
int *g_shmVector;

void define_shmVector() {
    g_shmVector = malloc(sizeof(int) * MAX_MESSAGES_PER_IPC);
    for (int i = 0; i < MAX_MESSAGES_PER_IPC; i++) {
        g_shmVector[i] = 0;
    }
}

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
}

void read_from_file(int fd, char *buffer, int size) {
    if (read(fd, buffer, size) == -1) {
        ErrExit("read failed");
    }
    buffer[size] = '\0';
}

void read_message(int fd, message_struct *m, int size) {
    if (read(fd, m, size) == -1) {
        ErrExit("read failed");
    }
}

// void write_shdm(message_struct *dest, message_struct *source) {
//     int index = 0;
//     while (g_shmVector[index] == 1) {
//         index++;
//         if (index == MAX_MESSAGES_PER_IPC) {
//             index = 0;
//         }
//     }
//     dest[index] = *source;
//     g_shmVector[index] = 1;
// }

// void read_shdm(message_struct *dest, message_struct *source) {
//     int index = 0;
//     while (g_shmVector[index] == 0) {
//         index++;
//         if (index == MAX_MESSAGES_PER_IPC) {
//             index = 0;
//         }
//     }
//     *dest = source[index];
//     g_shmVector[index] = 0;
// }