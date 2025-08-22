
#ifndef UTILS_H

#define UTILS_H

#include <stdio.h>
#include <unistd.h>             //è necessaria per usare getruid()
#include <arpa/inet.h>          //necessaria per usare le funzioni di rete
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>         //necessaria per le funzioni di socket
#include <netinet/in.h>         //necessaria per le strutture di rete
#include <netdb.h>              //necessaria per le funzioni di risoluzione DNS
#include <sys/types.h>          //necessaria per estendere i tipi di dato

int check_ipv4(char *ip, struct in_addr *ip_bin);
void store_ip(char *ip, struct in_addr *ip_bin, struct in6_addr *ip_bin6, int flag);
char *dns_resolver_ipv4(char *url);
char *reverse_dns(struct in_addr ip_bin);
double gettimestamp();
int check_ipv6(char *ip, struct in6_addr *ip_bin);
char *dns_resolver_ipv6(char *url);
char *reverse_dns_ipv6(struct in6_addr ip_bin);



#endif 