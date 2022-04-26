/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del client

#pragma once

#include <stdio.h>
#include <limits.h>

#define MAX_FILES 100


// 2d array to contain the path of MAX_FILES files
char g_files[MAX_FILES][NAME_MAX];
char *g_wd;

void start_client0();
void terminate_client0();
void block_all_signals();
char * get_username();
size_t append_path(char *);
int check_file_name(char *, char *);
int check_file_size(char *, off_t);
int check_num_chars_in_file(int);
void search(int);
int get_num_files();