
#ifndef PRINT_H

#define PRINT_H

#include <stdio.h>
#include <unistd.h> //è necessaria per usare getruid()
#include <arpa/inet.h> //necessaria per usare le funzioni di rete
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h> //necessaria per le funzioni di socket
#include <netinet/in.h> //necessaria per le strutture di rete
#include <netdb.h> //necessaria per le funzioni di risoluzione DNS
#include <sys/types.h> //necessaria per estendere i tipi di dato
#include "struct.h" //includo il file header per le dichiarazioni delle strutture



void print_line(informations *array, int hop_number);
void print_intro(char *ip, char *url);
void print_numbers(int hop_number);
void print_final(informations end); 

#endif