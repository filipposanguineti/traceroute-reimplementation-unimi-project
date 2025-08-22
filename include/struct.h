
#ifndef STRUCT_H

#define STRUCT_H

#include <stdio.h>
#include <unistd.h>             //è necessaria per usare getruid()
#include <arpa/inet.h>          //necessaria per usare le funzioni di rete
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>         //necessaria per le funzioni di socket
#include <netinet/in.h>         //necessaria per le strutture di rete
#include <netdb.h>              //necessaria per le funzioni di risoluzione DNS
#include <sys/types.h>          //necessaria per estendere i tipi di dato



#define BUFFER_SIZE 1500


//strutture dati
typedef struct {
    char ip_string[INET_ADDRSTRLEN]; 
    double rtt; 
    char url[BUFFER_SIZE];
} informations;                 //è la struttura che passo alla funzione di stampa

typedef struct {
    char ip_string[INET6_ADDRSTRLEN]; 
    double rtt; 
    char url[BUFFER_SIZE];
} informations_ipv6;                 //è la struttura che passo alla funzione di stampa per ipv6


#endif