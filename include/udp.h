
#ifndef UDP_H

#define UDP_H

#include <stdio.h>
#include <unistd.h> //è necessaria per usare getruid()
#include <arpa/inet.h> //necessaria per usare le funzioni di rete
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h> //necessaria per le funzioni di socket
#include <netinet/in.h> //necessaria per le strutture di rete
#include <netdb.h> //necessaria per le funzioni di risoluzione DNS
#include <sys/types.h> //necessaria per estendere i tipi di dato


int create_socket_udp();
int ttl_increment(int sd, int ttl);
int send_probe(int sd, struct in_addr ip_bin, int ttl, int probe_index);
void stampa_ttl_test(int sd);
int close_socket_udp(int sd);





#endif