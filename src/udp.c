
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

//dichiarazioni funzioni



int create_socket_udp(){

    int sd = socket(AF_INET, SOCK_DGRAM, 0); //creo una socket UDP
    if(sd < 0) {
        fprintf(stderr, "Error creating UDP socket.\n");
        return -1; 
    }

    printf("UDP socket created successfully: %d\n", sd);
    return sd;

}

int ttl_increment(int sd, int ttl){

    //per modificare il ttl posso usare la funzione setsockopt
    //passo quindi il socket descriptor, il protocollo da modificare (IP), il campo del pacchetto (ttl), il valore che voglio, e la dimensione che ha (un intero)
    int status = setsockopt(sd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

    if(status < 0) {
        fprintf(stderr, "Error setting TTL.\n");
        return -1; 
    }

    return 0;
}

void stampa_ttl_test(int sd){
    int current_ttl;
    socklen_t optlen = sizeof(current_ttl);
    if (getsockopt(sd, IPPROTO_IP, IP_TTL, &current_ttl, &optlen) == -1) {
        perror("getsockopt IP_TTL");
    } else {
        printf("TTL effettivo sulla socket: %d\n", current_ttl);
        return;
    }
}




int close_socket_udp(int sd){

    int status = close(sd); 
    if(close < 0) {
        fprintf(stderr, "Error closing UDP socket.\n");
        return -1; //ritorno -1 in caso di errore
    }

    return 0; //ritorno 0 se la chiusura è andata a buon fine
}