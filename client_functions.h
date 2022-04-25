/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del client

#pragma once

#include <stdio.h>
#include <limits.h>

#define MAX_FILES 100


void start_client0();
void terminate_client0();
void block_all_signals();
char * get_username();
size_t append_path(char *);
int check_file_name(char *, char *);
int check_file_size(char *, off_t);
void search(char [][NAME_MAX], int);
int get_num_files(char [][NAME_MAX]);