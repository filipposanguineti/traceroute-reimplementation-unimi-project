
#define _POSIX_C_SOURCE 200112L // Definisce macro per POSIX, necessaria per alcune funzioni di rete
#define GNU_SOURCE 1// Definisce macro per GNU, necessaria per alcune funzioni di rete
#define _DEFAULT_SOURCE 1 // Definisce macro per le funzioni di rete, necessaria per compatibilità con C17
#include <stdio.h>
#include <unistd.h> //è necessaria per usare geteuid()
#include <arpa/inet.h> //necessaria per usare le funzioni di rete
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h> //necessaria per le funzioni di socket
#include <netinet/in.h> //necessaria per le strutture di rete
#include <netdb.h> //necessaria per le funzioni di risoluzione DNS
#include <sys/types.h> //necessaria per estendere i tipi di dato
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>


#include "utils.h" //includo il file header per le dichiarazioni delle funzioni

#define BUFFER_SIZE 1500 // Definisco una costante per la dimensione del buffer



int create_socket_raw_icmp(){

    int sd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if(sd < 0) {
        fprintf(stderr, "Error creating raw ICMP socket.\n");
        return -1; 

    }else {
        printf("Raw ICMP socket created successfully: %d\n", sd);
        return sd;
    }
    //la bind non serve perché icmp non opera sulle porte (è a livello 3)
}


int receive_icmp(int sd, char *buffer){

    struct sockaddr_in reply_addr; //struttura vuota che conterrà l'indirizzo di chi mi ha risposto
    socklen_t addr_len = sizeof(reply_addr); //lunghezza della struttura (per la recvfrom mi serve il tipo socklen_t)

    int received = recvfrom(sd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&reply_addr, &addr_len); //passo sd, il buffer, la sua dimensione, il flag 0 per nessuna opzione, indirizzo vuoto creato prima e la sua dimensione

    if(received < 0) {
        fprintf(stderr, "Error receiving ICMP packet.\n");
        return -1; //ritorno -1 in caso di errore
    }else {
        printf("ICMP packet received from %s\n", inet_ntoa(reply_addr.sin_addr)); //stampo l'indirizzo di chi mi ha risposto
        return 0;
    }

    //il buffer dovrebbe essere pieno e può essere gestito dalla funzione in carica
}


int extract_rec_data(char *data, struct in_addr *addr, int *error){

    //devo recuperare l'ip, e l'error code dal pacchetto ricevuto, e la porta usata per distinguere il probe giusto

    //l'header completo è molto grande
    //primi 20byte è l'header IP esterno (quello da cui si recupera l'indirizzo)
    //poi 8 byte dell'header ICMP (quello da cui estraggo il type e il code di errore)
    //poi 20byte di header ip interno (quello del probe iniziale)
    //inifne 8 byte dell'header udp del probe iniziale (da cui estraggo la porta)

    //mi devo muovere in ordine, saltando da un header all'altro fino ad arrivare in fondo

    //estrazione ip
    //dentro la struct iphdr c'è il campo saddr che contiene l'indirizzo IP del mittente, il valore è big-endian 
    struct iphdr *ip_header = (struct iphdr *) data; //devo castare a iphdr per poter leggere correttamente i campi
    addr->s_addr = ip_header->saddr; //metto l'ip dentro s-addr perché è il campo che lo deve contenere (diverso da inet_pton a cui si passa semplicemente la struct in_addr)

}


int close_socket_icmp(int sd){

    int status = close(sd); 
    if(status < 0) {
        fprintf(stderr, "Error closing ICMP socket.\n");
        return -1; //ritorno -1 in caso di errore

    }else {
        printf("ICMP socket closed successfully.\n");
        return 0;
    }

}