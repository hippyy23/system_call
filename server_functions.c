/// @file server_functions.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per il server.

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <sys/stat.h>

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

void write_messages_to_files(int numFiles) {
    int fd, f2Index, msgqIndex, shdmIndex;
    char newPath[NAME_MAX];
    char mode[][9] = {"FIFO1", "FIFO2", "MsgQueue", "ShdMem"};

    for (int index = 0; index < numFiles; index++) {
        strcpy(newPath, container_fifo1[index].path);
        strcat(newPath, "_out");
        printf("<Server> creating file %s...\n", newPath);
        fd = open_file(newPath);
        write_to_file(fd, &container_fifo1[index], 1, mode[0]);

        f2Index = get_index(&container_fifo1[index], container_fifo1[index].pid, numFiles);
        write_to_file(fd, &container_fifo2[f2Index], 2, mode[1]);

        msgqIndex = get_index(&container_msgq[index].mtext, container_msgq[index].mtext.pid, numFiles);
        write_to_file_msgq(fd, &container_msgq[msgqIndex], 3, mode[2]);

        // shdmIndex = get_index(&container_shdm[index], container_shdm[index].pid, numFiles);
        // write_to_file(fd, &container_shdm[shdmIndex], 4, mode[3]);

        close(fd);
    }
}

int open_file(char *path) {
    int fd = open(path, O_WRONLY | O_APPEND | O_CREAT, S_IWUSR, S_IRUSR);
    if (fd == -1) {
        ErrExit("open failed");
    }

    return fd;
}

int get_index(message_struct *container, int pid, int size) {
    int index;

    for (index = 0; index < size; index++) {
        if (container[index].pid == pid) {
            return index;
        }
    }

    return -1;
}

void write_to_file(int fd, message_struct *m, int section, char mode[]) {
    char buffer[sizeof(message_struct) + 50];

    int n = sprintf(buffer, "[Parte %d, del file %s, spedita dal processo %d tramite %s]\n%s\n\n",
            section, m->path, m->pid, mode, m->content);
    int bR = write(fd, buffer, n);
    if (bR == -1) {
        ErrExit("write failed");
    }
}

void write_to_file_msgq(int fd, msgqueue_struct *m, int section, char mode[]) {
    char buffer[sizeof(message_struct) + 50];

    int n = sprintf(buffer, "[Parte %d, del file %s, spedita dal processo %d tramite %s]\n%s\n\n",
            section, m->mtext.path, m->mtext.pid, mode, m->mtext.content);
    int bR = write(fd, buffer, n);
    if (bR == -1) {
        ErrExit("write failed");
    }
}