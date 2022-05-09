/// @file shared_memory.h
/// @brief Contiene la definizioni di variabili e funzioni
///         specifiche per la gestione della MEMORIA CONDIVISA.

#pragma once

#include "defines.h"


int alloc_shared_memory(key_t, size_t);
void * attach_shared_memory(int, int);
void free_shared_memory(void *);
void read_shdm(message_struct *, message_struct *, short *);
int write_shdm(message_struct *, message_struct *, short *);
void remove_shared_memory(int);