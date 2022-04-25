/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once

#include <stdio.h>
#include <limits.h>

#define MAX_FILES 100
#define MAX_FILE_SIZE 4096
#define SHDMEM_SIZE 51200


// GLOBAL VARIABLES

char *g_wd;
char *g_fifo1;
char *g_fifo2;
int g_msgKey;
int g_shmKey;
int g_semKey;
