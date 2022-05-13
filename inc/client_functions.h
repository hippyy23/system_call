/// @file client_functions.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del client

#pragma once

#include <stdio.h>
#include <limits.h>

#include "defines.h"

#define MAX_FILES 100


// 2d array to contain the path of 'MAX_FILES' files
char g_files[MAX_FILES][PATH_SIZE];
// pointer to the cwd
char *g_wd;
sigset_t signalSet;
sigset_t oldSet;

void start_client0();
void terminate_client0();
void block_all_signals();
void unlock_signals();
size_t append_path(char *);
int check_file_name(char *, char *);
int check_file_size(char *, off_t);
int check_num_chars_in_file(int);
void search(int *);
void read_from_file(int, char *, int);
message_struct create_message_struct(int, int, int, int);
msgqueue_struct create_msgqueue_struct(int, int, int, int);
