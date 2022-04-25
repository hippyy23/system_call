/// @file shared_memory.h
/// @brief Contiene la definizioni di variabili e funzioni
///         specifiche per la gestione della MEMORIA CONDIVISA.

#pragma once


int alloc_shared_memory(key_t, size_t);
void * attach_shared_memory(int, int);
void free_shared_memory(void *);
void remove_shared_memory(int);