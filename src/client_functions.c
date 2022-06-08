/// @file client_functions.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per il client.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/msg.h>

#include "client_functions.h"
#include "defines.h"
#include "err_exit.h"


/**
 * @brief Block all signals
 */
void block_all_signals() {
    // add removed signals to the set
    sigaddset(&signalSet, SIGINT);
    sigaddset(&signalSet, SIGUSR1);    
    // update the mask with the signals to be blocked
    sigprocmask(SIG_BLOCK, &signalSet, &oldSet);
    printf("<Client_0> all signals blocked!\n");
}

/**
 * @brief Restore the old mask
 */
void unlock_signals() {
    // update the mask
    sigprocmask(SIG_SETMASK, &oldSet, NULL);
}

/**
 * @brief Terminate the client when SIGUSR1 is recieved
 * 
 * @param sig int
 */
void terminate_client0(int sig) {
    printf("<Client_0> Quitting...\n");
    exit(0);
}

/**
 * @brief Set the cwd to the parameter passed through console
 * 
 * @param sig int
 */
void start_client0(int sig) {
    printf("<Client_0> Starting...\n");

    // change the current working directory
    if (chdir(g_wd) == -1)
        ErrExit("change directory failed");
    // get the current directory
    char cwdbuf[PATH_MAX];
    getcwd(cwdbuf, PATH_MAX);

    printf("Ciao %s, ora inizio l'inivio dei file contenuti in %s\n", getenv("USER"), cwdbuf);
};

/**
 * @brief Search for files that start with "sendme_" in the cwd and all sub-directories
 */
void search(int *pos) {
    DIR *dirp = opendir(g_wd);
    if (dirp == NULL) {
        ErrExit("open dir failed");
    }

    errno = 0;
    struct dirent *dentry;
    while ((dentry = readdir(dirp)) != NULL) {
        if (strcmp(dentry->d_name, ".") == 0 || strcmp(dentry->d_name, "..") == 0) {
            continue;
        }
        if (dentry->d_type == DT_REG) {
            size_t lastPath = append_path(dentry->d_name);
            if (check_file_name(dentry->d_name, "sendme_") && check_file_size(g_wd, MAX_FILE_SIZE)) {
                strncpy(g_files[*pos], g_wd, PATH_SIZE);
                (*pos)++;
                if (*pos > MAX_FILES) {
                    printf("too many files: check MAX_FILES\n");
                    exit(0);
                }
            }            
            g_wd[lastPath] = '\0';
        } else if (dentry->d_type == DT_DIR) {
            size_t lastPath = append_path(dentry->d_name);
            search(pos);
            g_wd[lastPath] = '\0';
        }
        errno = 0;
    }
    if (errno != 0) {
        ErrExit("readdir failed");
    }
    if (closedir(dirp) == -1) {
        ErrExit("closedir failed");
    }
}

size_t append_path(char *dir) {
    size_t lastPathEnd = strlen(g_wd);
    strcat(strcat(&g_wd[lastPathEnd], "/"), dir);

    return lastPathEnd;
}

/**
 * @brief Check if first 7 chars of the file name 
 *        are equal to the string passed as parameter
 * 
 * @param file char*
 * @param str char*
 * @return int 
 */
int check_file_name(char *filename, char *str) {
    return (filename == NULL || str == NULL) ?
        0 : strncmp(filename, str, 7) == 0 && strstr(filename, "_out") == NULL;
}

/**
 * @brief Check if the file size if less or equal
 *        to the size passed as parameter
 * 
 * @param pathname char*
 * @param size off_t
 * @return int 
 */
int check_file_size(char *pathname, off_t size) {
    if (pathname == NULL) {
        return 0;
    }
    struct stat statbuf;
    if (stat(pathname, &statbuf) == -1) {
        return 0;
    }
    int fileSize = statbuf.st_size;

    return fileSize <= size && fileSize >= 4;
}

/**
 * @brief Check the number of chars in file
 * 
 * @param fd int
 * @return int
 */
int check_num_chars_in_file(int fd) {
    int count = 0;
    ssize_t bR;
    char buffer;
    do {
        bR = read(fd, &buffer, sizeof(char));
        if (bR == -1) {
            ErrExit("read failed\n");
        } else if (bR > 0) {
            count++;
        }
    } while (bR > 0);
    lseek(fd, 0, SEEK_SET);

    return count;
}

/**
 * @brief Read content from file
 * 
 * @param fd int
 * @param buffer char* 
 * @param size int
 */
void read_from_file(int fd, char *buffer, int size) {
    int n = read(fd, buffer, size);
    if (n == -1) {
        ErrExit("read failed");
    }
    buffer[n] = '\0';
}

/**
 * @brief Create a message struct object
 * 
 * @param fileFD int
 * @param pid pid_t
 * @param index int
 * @param size int
 * @return message_struct 
 */
message_struct create_message_struct(int fileFD, pid_t pid, int index, int size) {
    // initialize the struct of the message
    message_struct m;
    // write the pid into the message
    m.pid = pid;
    // write the path into the message
    strncpy(m.path, g_files[index], PATH_SIZE);
    // read from file a chunk of size 'size' and write it into the message
    read_from_file(fileFD, m.content, size);
    return m;
}

/**
 * @brief Create a msgqueue struct object
 * 
 * @param fileFD int
 * @param pid pid_t
 * @param index int
 * @param size int
 * @return msgqueue_struct
 */
msgqueue_struct create_msgqueue_struct(int fileFD, pid_t pid, int index, int size) {
    // initialize the struct of the message
    msgqueue_struct m;
    m.mtype = 1;
    // write the pid into the message
    m.mtext.pid = pid;
    // write the path into the message
    strncpy(m.mtext.path, g_files[index], PATH_SIZE);
    // read from file a chunk of size 'size' and write it into the message
    read_from_file(fileFD, m.mtext.content, size);

    return m;
}
