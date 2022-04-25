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

#include "client_functions.h"
#include "defines.h"
#include "err_exit.h"


void block_all_signals() {
    // set of signals
    sigset_t signalSet;
    // initialize signalSet
    sigfillset(&signalSet);
    // block all signals
    sigprocmask(SIG_SETMASK, &signalSet, NULL);
    printf("<Client_0> all signals blocked!\n");
}

void terminate_client0() {
    printf("<Client_0> Quitting...\n");
    _exit(0);
}

void start_client0() {
    printf("<Client_0> Starting...\n");
    block_all_signals();

    // change the current working directory
    if (chdir(g_wd) == -1)
        ErrExit("change directory failed");
    // get the current directory
    char cwdbuf[PATH_MAX];
    getcwd(cwdbuf, PATH_MAX);

    printf("Ciao %s, ora inizio l'inivio dei file contenuti in %s\n", get_username(), cwdbuf);
};

char * get_username() {
    struct passwd *pwd = getpwuid(getuid());
    if (pwd) {
        return pwd->pw_name;
    } else {
        return "";
    }
}

void search(char files[MAX_FILES][NAME_MAX], int pos) {
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

            if (check_file_name(dentry->d_name, "send_") &&
                check_file_size(g_wd, MAX_FILE_SIZE)) {
                strncpy(files[pos], g_wd, NAME_MAX);
                // TO BE REMOVED
                printf("%s\n", files[pos]);
                pos++;
                
                if (pos > MAX_FILES) {
                    ErrExit("too many files: check MAX_FILES");
                }
            }
            
            g_wd[lastPath] = '\0';
        } else if (dentry->d_type == DT_DIR) {
            size_t lastPath = append_path(dentry->d_name);
            search(files, pos);
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
            0 : strncmp(file, str, 5);
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

int get_num_files(char files[MAX_FILES][NAME_MAX]) {
    int n = 0;

    for (int i = 0; strcmp(files[i], ""); i++) {
        n++;
    }

    return n;
}
