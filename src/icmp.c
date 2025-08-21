
#define _POSIX_C_SOURCE 200112L     // Definisce macro per POSIX, necessaria per alcune funzioni di rete
#define GNU_SOURCE 1                // Definisce macro per GNU, necessaria per alcune funzioni di rete
#define _DEFAULT_SOURCE 1           // Definisce macro per le funzioni di rete, necessaria per compatibilità con C17
#include <stdio.h>
#include <unistd.h>                 //è necessaria per usare geteuid()
#include <arpa/inet.h>              //necessaria per usare le funzioni di rete
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>             //necessaria per le funzioni di socket
#include <netinet/in.h>             //necessaria per le strutture di rete
#include <netdb.h>                  //necessaria per le funzioni di risoluzione DNS
#include <sys/types.h>              //necessaria per estendere i tipi di dato
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>


#include "utils.h"                  //includo il file header per le dichiarazioni delle funzioni

#define BUFFER_SIZE 1500            // Definisco una costante per la dimensione del buffer



int create_socket_raw_icmp(){

    int sd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if(sd < 0) {

        fprintf(stderr, "Error creating raw ICMP socket.\n");
        return -1; 

    }else {

        return sd;
    }
    
    //la bind non serve perché icmp non opera sulle porte (è a livello 3)

}


int receive_icmp(int sd, char *buffer, int flag){

    int received;

    if(flag == 4){

        struct sockaddr_in reply_addr;              //struttura vuota che conterrà l'indirizzo di chi mi ha risposto
        socklen_t addr_len = sizeof(reply_addr);    //lunghezza della struttura (per la recvfrom mi serve il tipo socklen_t)

        //passo sd, il buffer, la sua dimensione, il flag 0 per nessuna opzione, indirizzo vuoto creato prima e la sua dimensione
        received = recvfrom(sd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&reply_addr, &addr_len); 

    }else if(flag == 6){

        struct sockaddr_in6 reply_addr;              
        socklen_t addr_len = sizeof(reply_addr);    

        received = recvfrom(sd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&reply_addr, &addr_len); 

        char ip_str[INET6_ADDRSTRLEN];
        printf("ICMP 6 reply: %s\n", inet_ntop(AF_INET6, &(reply_addr.sin6_addr), ip_str, sizeof(ip_str)));
        


    }

    

    if(received < 0) {

        fprintf(stderr, "Error receiving ICMP packet.\n");
        return -1;

    }else {
        
        return 0;
    }

    //il buffer dovrebbe essere pieno e può essere gestito dalla funzione di estrazione
}


int extract_rec_data(char *data, struct in_addr *addr, char *addr_string, int *error, int *port){

    //devo recuperare l'ip, e l'error code dal pacchetto ricevuto, e la porta usata per distinguere il probe giusto

    //l'header completo è molto grande
    //primi 20byte è l'header IP esterno (quello da cui si recupera l'indirizzo)
    //poi 8 byte dell'header ICMP (quello da cui estraggo il type e il code di errore)
    //poi 20byte di header ip interno (quello del probe iniziale)
    //inifne 8 byte dell'header udp del probe iniziale (da cui estraggo la porta)

    //mi devo muovere in ordine, saltando da un header all'altro fino ad arrivare in fondo



    //ESTRAZIONE IP

    //dentro la struct iphdr c'è il campo saddr che contiene l'indirizzo IP del mittente, il valore è big-endian 

    struct iphdr *ip_header = (struct iphdr *) data;    //devo castare a iphdr per poter leggere correttamente i campi
    addr->s_addr = ip_header->saddr;                    //metto l'ip dentro s-addr perché è il campo che lo deve contenere (diverso da inet_pton a cui si passa semplicemente la struct in_addr)

    if(inet_ntop(AF_INET, addr, addr_string, INET_ADDRSTRLEN) == NULL) { 
           //converto l'indirizzo IP binario in stringa
        fprintf(stderr, "Error converting binary IP to string.\n");
        return -1;
    
    }
    


    //ESTRAZIONE TYPE E CODE

    //per trovare l'inizio dell'header icmp, devo saltare oltre quello ip. Devo recuperare il campo ihl dell'header ip e moltiplicarlo per 4 per esprimerlo in bytes (perché conta con 32bit ovvero 4 bytes)

    int ip_lenght = ip_header->ihl * 4; 

    struct icmphdr *icmp_header = (struct icmphdr *) (data + ip_lenght);    //devo castare a icmphdr per poter leggere correttamente i campi, sommando data all'header ip arrivo a quello icmp
    int type = icmp_header->type;                                           //estraggo il type
    *error = icmp_header->code;                                             //estraggo il code
    


    //ESTRAZIONE PORTA

    //siccome prima di udp c'è un altro header ip (quello mio originale del probe), devo calcolarne la lunghezza e superarlo. Per superare icmp basta aggiungere 8 byte (icmp ha una lunghezza fissa)

    struct iphdr *ip_header_probe = (struct iphdr *) (data + ip_lenght + sizeof(struct icmphdr)); 

    int ip_probe_length = ip_header_probe->ihl * 4;                         //calcolo la lunghezza dell'header ip del probe

    //ora posso andare all'header udp
    struct udphdr *udp_header = (struct udphdr *) (data + ip_lenght + sizeof(struct icmphdr) + ip_probe_length); //uso il sizeof al posto di 8 byte

    *port = ntohs(udp_header->dest); //estraggo la porta di destinazione, che è quella del probe, e la converto da big endian a little endian


    return 0;

}


int close_socket_icmp(int sd){

    int status = close(sd); 
    if(status < 0) {
        fprintf(stderr, "Error closing ICMP socket.\n");
        return -1; 

    }else {
        
        return 0;
    }

}



//IPV6

int create_socket_raw_icmp_ipv6(){

    int sd = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);    //af_inet6 e icmpv6

    if(sd < 0) {

        fprintf(stderr, "Error creating raw ICMP IPv6 socket.\n");
        return -1; 

    }else {

        return sd;
    }
    
    
}







//MEMO SULLA STRUTTURA DEI PACCHETTI RICEVUTI (IPV4)

// [0]  IP header esterno (struct iphdr) - 20 byte min. (ihl*4)
//      Campo         | Dimensione | Descrizione
//      --------------+------------+--------------------------------
//      version       | 4 bit      | Versione IP (4 per IPv4)
//      ihl           | 4 bit      | Lunghezza header IP in parole da 32 bit
//      tos           | 1 byte     | Type of Service
//      tot_len       | 2 byte     | Lunghezza totale pacchetto IP
//      id            | 2 byte     | Identificatore
//      frag_off      | 2 byte     | Offset di frammento + flag DF/MF
//      ttl           | 1 byte     | Time To Live residuo
//      protocol      | 1 byte     | Protocollo (1 = ICMP)
//      check         | 2 byte     | Checksum header IP
//      saddr         | 4 byte     | IP sorgente (router o destinazione)
//      daddr         | 4 byte     | IP destinazione (te)

// [iphdr->ihl*4]  ICMP header (struct icmphdr) - 8 byte
//      Campo         | Dimensione | Descrizione
//      --------------+------------+--------------------------------
//      type          | 1 byte     | Tipo messaggio ICMP
//      code          | 1 byte     | Sotto-tipo/dettaglio
//      checksum      | 2 byte     | Checksum ICMP
//      rest_of_hdr   | 4 byte     | Varia in base al tipo (es. unused/MTU/pointer)

// [+8]  IP header originale (struct iphdr) - 20 byte min.
//      Campo         | Dimensione | Descrizione
//      --------------+------------+--------------------------------
//      version       | 4 bit
//      ihl           | 4 bit
//      tos           | 1 byte
//      tot_len       | 2 byte
//      id            | 2 byte
//      frag_off      | 2 byte
//      ttl           | 1 byte
//      protocol      | 1 byte     | (17 = UDP nel traceroute)
//      check         | 2 byte
//      saddr         | 4 byte     | IP sorgente originale (io)
//      daddr         | 4 byte     | IP destinazione originale

// [ip_orig->ihl*4 dopo questo punto]  UDP header originale (struct udphdr) - 8 byte
//      Campo         | Dimensione | Descrizione
//      --------------+------------+--------------------------------
//      source        | 2 byte     | Porta sorgente (7777)
//      dest          | 2 byte     | Porta destinazione (33434 + ttl + probe)
//      len           | 2 byte     | Lunghezza totale UDP (header+payload)
//      check         | 2 byte     | Checksum UDP


