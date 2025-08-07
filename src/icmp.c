
#define _POSIX_C_SOURCE 200112L // Definisce macro per POSIX, necessaria per alcune funzioni di rete
#include <stdio.h>
#include <unistd.h> //è necessaria per usare geteuid()
#include <arpa/inet.h> //necessaria per usare le funzioni di rete
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h> //necessaria per le funzioni di socket
#include <netinet/in.h> //necessaria per le strutture di rete
#include <netdb.h> //necessaria per le funzioni di risoluzione DNS
#include <sys/types.h> //necessaria per estendere i tipi di dato

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