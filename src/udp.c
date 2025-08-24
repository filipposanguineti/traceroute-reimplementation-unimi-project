
#define _POSIX_C_SOURCE 200112L         // Definisce macro per POSIX, necessaria per alcune funzioni di rete
#include <stdio.h>
#include <unistd.h>                     //è necessaria per usare geteuid()
#include <arpa/inet.h>                  //necessaria per usare le funzioni di rete
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>                 //necessaria per le funzioni di socket
#include <netinet/in.h>                 //necessaria per le strutture di rete
#include <netdb.h>                      //necessaria per le funzioni di risoluzione DNS
#include <sys/types.h>                  //necessaria per estendere i tipi di dato
#include <netinet/udp.h>


#include "utils.h"                      





int create_socket_udp(){

    int sd = socket(AF_INET, SOCK_DGRAM, 0);                                        //creo una socket UDP, 0 sta per il protocollo predefinito
    if(sd < 0) {

        fprintf(stderr, "Error creating UDP socket.\n");
        return -1; 
    }


    //preparo la bind per assegnare una porta interna fissa per l'invio dei miei pacchetti
    struct sockaddr_in bind_addr; 
    memset(&bind_addr, 0, sizeof(bind_addr));

    bind_addr.sin_family = AF_INET;                                                 //imposto la famiglia di indirizzi
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);                                  //voglio usare una qualsiasi interfaccia (se metto 127.0.0.1 rimane solo in loopback), passo INADDR_ANY in big endian
    bind_addr.sin_port = htons(7777);                                               //metto la porta 7777 in big endian (tutte le comunicazioni passeranno da questa porta interna)

    int result = bind(sd, (struct sockaddr *)&bind_addr, sizeof(bind_addr));        //faccio la bind della socket all'indirizzo e alla porta

    if(result < 0) {

        fprintf(stderr, "Error binding UDP socket.\n");
        return -1;

    }

    return sd;

}



int ttl_increment(int sd, int ttl, int flag){

    int status;

    if(flag == 4){

        //per modificare il ttl posso usare la funzione setsockopt
        //passo quindi il socket descriptor, il protocollo da modificare (IP), il campo del pacchetto (ttl), il valore che voglio, e la dimensione che ha (un intero)
        status = setsockopt(sd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

    }else if(flag == 6){

        
        //cambai il protocllo, inoltre in ipv6 i ttl vengono chiamati hop, metto unicast perché mi interessa un'unica destinazione
        status = setsockopt(sd, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &ttl, sizeof(ttl));


    }

    

    if(status < 0) {

        fprintf(stderr, "Error setting TTL.\n");
        return -1; 

    }

    return 0;
}


int send_probe(int sd, struct in_addr ip_bin, int ttl, int probe_index, int *port){

    //creo la struttura sockaddr_in per il destinatario
    
    //la porta deve essere univoca per ogni probe di ogni ttl. Se sommassi solo base+ttl+index potrei avere delle sovrapposizioni
    //uso quindi una formula che usando la moltiplicazione assicura l'univocita della porta
    //base + (ttl-1)*numero_di_probe + probe_index (il ttl-1 serve per parire dalla base, ma non è necessario, va bene anche ttl)
    *port = 33434 + (ttl-1)*3 + probe_index;

    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest)); 

    dest.sin_family = AF_INET;                                                      //imposto la famiglia di indirizzi
    dest.sin_addr = ip_bin;                                                         //imposto l'indirizzo di destinazione
    dest.sin_port = htons(*port);                                                   //imposto la porta di destinazione con cui identificherò le risposte

    //mando il pacchetto, lo lascio vuoto per comodità
    int sent = sendto(sd, NULL, 0, 0, (struct sockaddr *)&dest, sizeof(dest));      //nessun buffer, quindi len 0, nessuna opzione quindi 0
    //sendto prende; sd, un buffer da inviare, la lunghezza del buffer, le opzioni, la destinazione, la dimensione della destinazion


    if(sent < 0) {

        fprintf(stderr, "Error sending probe.\n");
        return -1;

    }else {

        return 0;

    }

}



void stampa_ttl_test(int sd){

    int current_ttl;

    socklen_t optlen = sizeof(current_ttl);

    if (getsockopt(sd, IPPROTO_IP, IP_TTL, &current_ttl, &optlen) == -1) {

        fprintf(stderr, "Error getting TTL.\n");
        return;

    } else {

        return;
    }

}








int close_socket_udp(int sd){

    int status = close(sd); 

    if(status < 0) {

        fprintf(stderr, "Error closing UDP socket.\n");
        return -1; 

    }

    return 0; 
    
}




//IPV6

int create_socket_udp_ipv6(){

    int  sd = socket(AF_INET6, SOCK_DGRAM, 0);                                    //uso af_inet6 per ipv6

    if(sd < 0) {

        fprintf(stderr, "Error creating IPv6 UDP socket.\n");
        return -1; 
    }

    //la bind
    struct sockaddr_in6 bind_addr;
    memset(&bind_addr, 0, sizeof(bind_addr));

    bind_addr.sin6_family = AF_INET6;                                             
    bind_addr.sin6_addr = in6addr_any;                                          //in6addr_any è l'equivalenete di INADDR_ANY per ipv6, è già in big endian             
    bind_addr.sin6_port = htons(7777);      
    
    int result = bind(sd, (struct sockaddr *)&bind_addr, sizeof(bind_addr));    

    if(result < 0) {

        fprintf(stderr, "Error binding IPv6 UDP socket.\n");
        return -1;

    }

    return sd;


}


int send_probe_ipv6(int sd, struct in6_addr ip_bin, int ttl, int probe_index, int *port){

    //il funzionamento è lo stesso del send probe ipv4, ma devo usare le strutture ipv6

     *port = 33434 + (ttl-1)*3 + probe_index;

    struct sockaddr_in6 dest;
    memset(&dest, 0, sizeof(dest)); 

    dest.sin6_family = AF_INET6;                                                    //imposto la famiglia di indirizzi
    dest.sin6_addr = ip_bin;                                                        //imposto l'indirizzo di destinazione
    dest.sin6_port = htons(*port);                                                  //imposto la porta di destinazione con cui identificherò le risposte

    //mando il pacchetto, lo lascio vuoto per comodità
    int sent = sendto(sd, NULL, 0, 0, (struct sockaddr *)&dest, sizeof(dest));      //uguale a ipv4

    if(sent < 0) {

        fprintf(stderr, "Error sending probe.\n");
        return -1;

    }else {
        
        return 0;

    }

}