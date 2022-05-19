/// @file fifo.h
/// @brief Contiene la definizioni di variabili e
///         funzioni specifiche per la gestione delle FIFO.

#pragma once

#include "defines.h"


void make_fifo(char *);
int open_fifo(char *, int);
int read_fifo(int, message_struct *);
void write_fifo(int, message_struct *);
void close_fifo(int, char *);
