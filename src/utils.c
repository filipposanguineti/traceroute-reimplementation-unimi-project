
#define _POSIX_C_SOURCE 200112L // Definisce macro per POSIX, necessaria per alcune funzioni di rete

#include "utils.h"

#include <stdio.h>
#include <unistd.h> //è necessaria per usare getruid()
#include <arpa/inet.h> //necessaria per usare le funzioni di rete
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h> //necessaria per le funzioni di socket
#include <netinet/in.h> //necessaria per le strutture di rete
#include <netdb.h> //necessaria per le funzioni di risoluzione DNS
#include <sys/types.h> //necessaria per estendere i tipi di dato
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>




int check_ipv4(char *ip){

    struct in_addr buffer; //mi serve per permettere a inetpton di operare sull'ip con un buffer che ha la taglia di un ipv4

    int val = inet_pton(AF_INET, ip, &buffer); //converte l'ip in un formato binario e salva il status code in val

    //se val è 1: ip valido
    //se val è 0: ip non valido
    //se val è -1: errore

    return val;

}



void store_ip(char *ip, struct in_addr *ip_bin) {

    inet_pton(AF_INET, ip, ip_bin); //uso di nuovo inet_pton, ma ora salvo la variabile

    if(ip_bin == NULL) {
        fprintf(stderr, "Error, the address doesn't exist.\n");
        return;
    }
    
    return;

}





char *dns_resolver_ipv4(char *url){

    //creo le hints per aiutare la risoluzione (per specificare che voglio ipv4 e udp)
    struct addrinfo hints; 
    memset(&hints, 0, sizeof(hints)); //inizializzo a zero



    //ora devo popolare la struct
    hints.ai_family = AF_INET; //specifico ipv4
    hints.ai_socktype = SOCK_DGRAM; //specifico socket udp



    //ora posso usare getaddrinfo per risolvere l'url
    struct addrinfo *result;
    int error = getaddrinfo(url, NULL, &hints, &result);



    //controllo gli errori 
    if(error != 0) { //errore nella risoluzione
        fprintf(stderr, "Error resolving URL\n");
        return NULL; 
    }

    if(result == NULL) { //nessun risultato
        fprintf(stderr, "No results found for the URL.\n");
        return NULL;
    }



    //estraggo l'ip (si trova dentro result->ai_addr)
    struct sockaddr *addr_sporca = result->ai_addr; //prendo l'indirizzo, ma devo castarlo, se no non posso leggerlo, deve diventare sockaddr_in
    struct sockaddr_in *addr = (struct sockaddr_in *)addr_sporca; //casto a sockaddr_in, per poter estrarre l'ip
    struct in_addr ip_bin = addr->sin_addr; //estraggo l'ip reale in binario



    //converto il binario in stringa
    char buffer[INET_ADDRSTRLEN]; //buffer temporaneo per la conversione da binario a stringa

    if(inet_ntop(AF_INET, &ip_bin, buffer, sizeof(buffer)) == NULL) { //converto da binario a stringa
        fprintf(stderr, "Error converting binary IP to string.\n");
        freeaddrinfo(result); //libero la memoria allocata da getaddrinfo
        return NULL;   
    }

    char *ip_string = malloc(strlen(buffer) + 1); //devo allocarlo dinamicamente per forza, se no nella return non viene salvato e la variabile si autodistrugge
    strcpy(ip_string, buffer); //copio il buffer nella stringa dinamica


    printf("Resolved URL %s to IP: %s\n", url, ip_string); //stampo l'ip risolto

    freeaddrinfo(result); //libero la memoria allocata da getaddrinfo

    return ip_string; //ritorno una copia della stringa dell'ip risolto, se non uso strdup, il puntatore viene perso e non posso più accedere alla stringa (avrei potuto allocarla dinamicamente prima) 







}