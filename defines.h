/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once

#include <stdio.h>

#define MAX_FILES 100
#define MAX_FILE_SIZE 4096

// GLOBAL VARIABLES

char **g_files;
char g_wd[255];
char fifo1[11];
char fifo2[11];


// CLIENT FUNCTIONS

void start();
void quit();
void block_all_signals();
char * get_username();
size_t append_path(char *);
int check_file_name(char *, char *);
int check_file_size(char *, off_t);
int search(int);
int get_num_files();
void free_mem_files();
void alloc_mem_files();

// SERVER FUNCTIONS
void make_fifo1(char *);