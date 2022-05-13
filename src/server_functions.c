/// @file server_functions.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per il server.

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/msg.h>

#include "server_functions.h"
#include "defines.h"
#include "err_exit.h"


/**
 * @brief Initialize containers for messages
 * 
 * @param numFiles int
 */
void initialize_space_for_msg(int numFiles) {
    container_fifo1 = malloc(sizeof(message_struct) * numFiles);
    container_fifo2 = malloc(sizeof(message_struct) * numFiles);
    container_msgq = malloc(sizeof(msgqueue_struct) * numFiles);
    container_shdm = malloc(sizeof(message_struct) * numFiles);

    if (container_fifo1 == NULL || container_fifo2 == NULL || 
    container_msgq == NULL || container_shdm == NULL) {
        ErrExit("malloc failed");
    } else {
        printf("<Server> allocated space for messages\n");
    }
}

/**
 * @brief Write recieved messages to files
 * 
 * @param numFiles int
 */
void write_messages_to_files(int numFiles) {
    int fd, f2Index, msgqIndex, shdmIndex, pid;
    char newPath[PATH_SIZE];
    char mode[][9] = {"FIFO1", "FIFO2", "MsgQueue", "ShdMem"};

    for (int index = 0; index < numFiles; index++) {
        // get PID of the message from FIFO1
        pid = container_fifo1[index].pid;
        set_file_out_name(container_fifo1[index].path, newPath);
        // create new file
        fd = create_file(newPath);
        // write to file content from FIFO1
        write_to_file(fd, &container_fifo1[index], 1, mode[0]);

        // get the message index that matches the PID
        f2Index = get_index(container_fifo2, pid, numFiles);
        // write to file content from FIFO2
        write_to_file(fd, &container_fifo2[f2Index], 2, mode[1]);

        // get the message index that matches the PID
        msgqIndex = get_index_msgq(container_msgq, pid, numFiles);
        // write to file content from MsgQueue
        write_to_file_msgq(fd, &container_msgq[msgqIndex], 3, mode[2]);

        // get the message index that matches the PID
        shdmIndex = get_index(container_shdm, pid, numFiles);
        // write to file content from ShdMem
        write_to_file(fd, &container_shdm[shdmIndex], 4, mode[3]);

        close(fd);
    }
}

/**
 * @brief Create a file
 * 
 * @param path char*
 * @return int 
 */
int create_file(char *path) {
    int fd = open(path, O_WRONLY | O_TRUNC | O_CREAT, S_IWUSR | S_IRUSR);
    if (fd == -1) {
        ErrExit("open failed");
    }

    return fd;
}

/**
 * @brief Set the file name for output files
 *        by adding _out to the end of the filename
 *        (the extension of the file is considered)
 * 
 * @param oldPath char*
 * @param newPath char*
 */
void set_file_out_name(char *oldPath, char *newPath) {
    // copy in a temp var the path to split
    char temp[PATH_SIZE];
    strcpy(temp, oldPath);

    // get path w/o file extension
    char *path = strtok(temp, ".");
    // get file extension
    char *extension = strtok(NULL, ".");
    // check if file has an extension
    if (extension == NULL) {
        snprintf(newPath, PATH_SIZE, "%s_out", path);
    } else {
        snprintf(newPath, PATH_SIZE, "%s_out.%s", path, extension);
    }
}

/**
 * @brief Get the message index that matches the PID from a container
 * 
 * @param container msgqueue_struct*
 * @param pid int
 * @param size int
 * @return int 
 */
int get_index(message_struct *container, int pid, int size) {
    for (int index = 0; index < size; index++) {
        if (container[index].pid == pid) {
            return index;
        }
    }

    return -1;
}

/**
 * @brief Get the message index that matches the PID from msq container
 * 
 * @param container msgqueue_struct*
 * @param pid int
 * @param size int
 * @return int 
 */
int get_index_msgq(msgqueue_struct *container, int pid, int size) {
    for (int index = 0; index < size; index++) {
        if (container[index].mtext.pid == pid) {
            return index;
        }
    }

    return -1;
}

/**
 * @brief Write content to file
 * 
 * @param fd int
 * @param m message_stuct 
 * @param section int
 * @param mode char*
 */
void write_to_file(int fd, message_struct *m, int section, char *mode) {
    char buffer[sizeof(message_struct) + 50];

    int n = sprintf(buffer, "[Parte %d, del file %s, spedita dal processo %d tramite %s]\n%s%s",
            section, m->path, m->pid, mode, m->content, section == 4 ? "\n" : "\n\n");
    int bR = write(fd, buffer, n);
    if (bR == -1) {
        ErrExit("write failed");
    }
}

/**
 * @brief Write content to file
 * 
 * @param fd int
 * @param m msgqueue_struct*
 * @param section int
 * @param mode char*
 */
void write_to_file_msgq(int fd, msgqueue_struct *m, int section, char *mode) {
    char buffer[sizeof(message_struct) + 50];

    int n = sprintf(buffer, "[Parte %d, del file %s, spedita dal processo %d tramite %s]\n%s\n\n",
            section, m->mtext.path, m->mtext.pid, mode, m->mtext.content);
    int bR = write(fd, buffer, n);
    if (bR == -1) {
        ErrExit("write failed");
    }
}
