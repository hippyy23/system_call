/// @file message_queue.h
/// @brief Contiene la definizioni di variabili e funzioni 
///         specifiche per la gestione della MESSAGE QUEUE.

#include "defines.h"

#pragma once


int create_msgq(int, int);
void read_msgq(int, msgqueue_struct *, int, int);
int write_msgq(int, msgqueue_struct *);
void remove_msgq(int);
