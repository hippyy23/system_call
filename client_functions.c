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


void block_all_signals() {
    // add removed signals to the set
    sigaddset(&signalSet, SIGINT);
    sigaddset(&signalSet, SIGUSR1);    
    // update the mask with the signals to be blocked
    sigprocmask(SIG_BLOCK, &signalSet, &oldSet);
    printf("<Client_0> all signals blocked!\n");
}

void unlock_signals() {
    // update the mask
    sigprocmask(SIG_SETMASK, &oldSet, NULL);
}

void terminate_client0(int sig) {
    printf("<Client_0> Quitting...\n");
    exit(0);
}

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

char * get_username() {
    struct passwd *pwd = getpwuid(getuid());
    if (pwd) {
        return pwd->pw_name;
    } else {
        return "";
    }
}

void search(int pos) {
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
                strncpy(g_files[pos], g_wd, NAME_MAX);
                // TO BE REMOVED
                //printf("%s\n", g_files[pos]);
                printf("%s\n", g_wd);
                pos++;
                
                if (pos > MAX_FILES) {
                    ErrExit("too many files: check MAX_FILES");
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

int check_file_name(char *file, char *str) {
    return (file == NULL || str == NULL) ?
        0 : strncmp(file, str, 7) == 0;
}

int check_file_size(char *pathname, off_t size) {
    if (pathname == NULL) {
        return 0;
    }

    struct stat statbuf;
    if (stat(pathname, &statbuf) == -1) {
        return 0;
    }

    return statbuf.st_size <= size;
}

int get_num_files() {
    int n = 0;

    for (int i = 0; strcmp(g_files[i], "") != 0; i++) {
        n++;
    }

    return n;
}

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

    return count - 1;
}

void write_fifo(int fifoFD, int fileFD, int pid, int index, int size) {
    message_struct m;
    m.pid = pid;
    strncpy(m.path, g_files[index], NAME_MAX);
    // read from file a chunk of size 'size'
    printf("<Client_%d> reading content from file...\n", pid);
    read_from_file(fileFD, m.content, size);

    if (write(fifoFD, &m, sizeof(m)) == -1) {
        ErrExit("write failed");
    }
}

void write_msgq(int msqid, int fileFD, int pid, int index, int size) {
    msgqueue_struct m;
    m.mtype = 1;
    m.mtext.pid = pid;
    strncpy(m.mtext.path, g_files[index], NAME_MAX);
    // read from file a chunk of size 'size'
    read_from_file(fileFD, m.mtext.content, size);

    // send the message to the server
    size_t mSize = sizeof(msgqueue_struct) - sizeof(long);
    if (msgsnd(msqid, &m, mSize, 0) == -1) {
        ErrExit("msgsnd failed");
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

void read_from_file(int fd, char *buffer, int size) {
    if (read(fd, buffer, size) == -1) {
        ErrExit("read failed");
    }
    buffer[size] = '\0';
}
