/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once

#include <stdio.h>
#include <limits.h>

#define MAX_FILES 100
#define MAX_FILE_SIZE 4096
#define SHDMEM_SIZE 51200
#define MAX_PAGE_SIZE 1024
#define MAX_MESSAGES_PER_IPC 50

#define START_END 0
#define WAIT_CHILD 1
#define MUTEX_SHM 2


struct message {
    char content[MAX_PAGE_SIZE];
    int pid;
    char path[NAME_MAX];
};

struct msgqueue {
    long mtype;
    struct message mtext;
};

// GLOBAL VARIABLES

char *g_fifo1;
char *g_fifo2;
int g_msgKey;
int g_shmKey;
int g_semKey;
int g_shmVector[MAX_MESSAGES_PER_IPC];
