/// @file semaphore.h
/// @brief Contiene la definizioni di variabili e funzioni
///         specifiche per la gestione dei SEMAFORI.

#pragma once


union semun {
    int val;
    struct semid_ds * buf;
    unsigned short * array;
};

int create_sem(key_t);
void semOp(int, unsigned short, short);