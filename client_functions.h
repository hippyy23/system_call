/// @file client_functions.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del client

#pragma once

#include <stdio.h>
#include <limits.h>

#include "defines.h"

#define MAX_FILES 100


// 2d array to contain the path of MAX_FILES files
char g_files[MAX_FILES][NAME_MAX];
char *g_wd;
int *g_shmVector;
sigset_t signalSet;
sigset_t oldSet;

void start_client0();
void terminate_client0();
void block_all_signals();
void unlock_signals();
char * get_username();
size_t append_path(char *);
int check_file_name(char *, char *);
int check_file_size(char *, off_t);
int check_num_chars_in_file(int);
void search(int);
int get_num_files();
void read_from_file(int, char *, int);
void write_fifo(int, int, int, int, int);
void write_shdm(message_struct *, message_struct *);
void write_msgq(int, int, int, int, int);
// void read_shdm(message_struct *, message_struct *);