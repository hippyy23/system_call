/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once

#include <stdio.h>
#include <limits.h>

#define MAX_FILES 100
#define MAX_FILE_SIZE 4096
#define MAX_PAGE_SIZE 1024
#define MAX_MESSAGES_PER_IPC 50
#define PATH_SIZE 150

#define WAIT_CHILD 0
#define MUTEX_SHM 1
#define LIMIT_FIFO1 2
#define LIMIT_FIFO2 3
#define LIMIT_MSGQ 4
#define SYNC_SHM 5
#define END 6


// STRUCT DEFINITION

typedef struct message {
    char content[MAX_PAGE_SIZE];
    int pid;
    char path[PATH_SIZE];
} message_struct;

typedef struct msgqueue {
    long mtype;
    struct message mtext;
} msgqueue_struct;


// GLOBAL VARIABLES

char *g_fifo1;
char *g_fifo2;
