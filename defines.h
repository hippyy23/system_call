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

#define START_END 0
#define WAIT_CHILD 1
#define MUTEX_SHM 2
#define SYNC_FIFO1 3
#define SYNC_FIFO2 4


typedef struct message {
    char content[MAX_PAGE_SIZE];
    int pid;
    char path[NAME_MAX];
} message_struct;

typedef struct msgqueue {
    long mtype;
    struct message mtext;
} msgqueue_struct;

// GLOBAL VARIABLES

char *g_fifo1;
char *g_fifo2;
int g_msgKey;
int g_shmKey;
int g_semKey;
extern int *g_shmVector;

int open_fifo(char *, int);
void define_shmVector();
void write_fifo(int, message_struct *, int);
void read_from_file(int, char *, int);
void read_message(int, message_struct *, int);
// void write_shdm(message_struct *, message_struct *);
// void read_shdm(message_struct *, message_struct *);