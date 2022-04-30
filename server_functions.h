/// @file server_functions.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del server

#pragma once


message_struct *container_fifo1;
message_struct *container_fifo2;
msgqueue_struct *container_msgq;
message_struct *container_shdm;

void initialize_space_for_msg(int);