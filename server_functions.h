/// @file server_functions.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del server

#pragma once

#include "defines.h"


message_struct *container_fifo1;
message_struct *container_fifo2;
msgqueue_struct *container_msgq;
message_struct *container_shdm;

int open_file(char *);
void initialize_space_for_msg(int);
void write_messages_to_files(int);
int get_index(message_struct *, int, int);
int get_index_msgq(msgqueue_struct *, int, int);
void read_fifo(int, message_struct *);
void read_shdm(message_struct *src, message_struct *dest, short *shmArr);
void write_to_file(int, message_struct *, int, char []);
void write_to_file_msgq(int, msgqueue_struct *, int, char []);
void remove_msgq(int);