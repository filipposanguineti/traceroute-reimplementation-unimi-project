
#ifndef ICMP_H

#define ICMP_H

#include <stdio.h>
#include <unistd.h> //è necessaria per usare getruid()
#include <arpa/inet.h> //necessaria per usare le funzioni di rete
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h> //necessaria per le funzioni di socket
#include <netinet/in.h> //necessaria per le strutture di rete
#include <netdb.h> //necessaria per le funzioni di risoluzione DNS
#include <sys/types.h> //necessaria per estendere i tipi di dato



int create_socket_raw_icmp();
int receive_icmp(int sd, char *buffer);
int close_socket_icmp(int sd);


#endif