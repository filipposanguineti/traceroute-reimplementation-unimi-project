
#define _POSIX_C_SOURCE 200112L         // Definisce macro per POSIX, necessaria per alcune funzioni di rete

#include "utils.h"

#include <stdio.h>
#include <unistd.h>                     //è necessaria per usare getruid()
#include <arpa/inet.h>                  //necessaria per usare le funzioni di rete
#include <string.h> 
#include <stdlib.h>
#include <sys/socket.h>                 //necessaria per le funzioni di socket
#include <netinet/in.h>                 //necessaria per le strutture di rete
#include <netdb.h>                      //necessaria per le funzioni di risoluzione DNS
#include <sys/types.h>                  //necessaria per estendere i tipi di dato
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <time.h>                       //necessaria per clock_gettime e timespec   
#include <netinet/in.h>                    

#define BUFFER_SIZE 1500                //definisco una costante per la dimensione del buffer




int check_ipv4(char *ip, struct in_addr *ip_bin){


    int val = inet_pton(AF_INET, ip, ip_bin);      //converte l'ip in un formato binario e salva il status code in val

    if(val == 1) {

        //devo salvare l'ip in una variabile (sia come stringa per la stampa, sia come binario per le socket)
        store_ip(ip, ip_bin, NULL, 4);   //salvo l'ip in formato binario, passando la variabile per argomento

        printf("Valid IPv4 address provided: %s\n", ip);

        
    
    }else if(val == 0) {

        //devo risolvere l'url in un ip
        printf("Not valid IPv4 address, resolving URL: %s\n", ip);

        char *resolved_ip = dns_resolver_ipv4(ip);     //risolvo l'url in un ip

        //salvo l'ip
        if(resolved_ip == NULL) {

            fprintf(stderr, "Error resolving URL to IP.\n");
            return 1;

        }

        store_ip(resolved_ip, ip_bin, NULL, 4);                     //salvo l'ip in formato binario, passando la variabile per argomento


        //alla fine libero la memoria di resolved_ip
        free(resolved_ip);

    }else{

        fprintf(stderr, "Error in checking IP address.\n");
        return 1;

    }

    //se val è 1: ip valido
    //se val è 0: ip non valido
    //se val è -1: errore

    return val;

}



void store_ip(char *ip, struct in_addr *ip_bin, struct in6_addr *ip_bin6, int flag) {


    if(flag == 4){                                      //flag 4 = ipv4

        inet_pton(AF_INET, ip, ip_bin);                 //uso di nuovo inet_pton, ma ora salvo la variabile
    
    }else if(flag == 6){                                //flag 6 = ipv6 

        inet_pton(AF_INET6, ip, ip_bin6);               

    }
    

    if(ip_bin == NULL && ip_bin6 == NULL) { 

        fprintf(stderr, "Error, the address doesn't exist.\n");
        return;

    }
    
    return;

}





char *dns_resolver_ipv4(char *url){

    //creo le hints per aiutare la risoluzione (per specificare che voglio ipv4 e udp)
    struct addrinfo hints; 
    memset(&hints, 0, sizeof(hints));                               //inizializzo a zero



    //ora devo popolare la struct
    hints.ai_family = AF_INET;                                      //specifico ipv4
    hints.ai_socktype = SOCK_DGRAM;                                 //specifico socket udp



    //ora posso usare getaddrinfo per risolvere l'url
    struct addrinfo *result;
    int error = getaddrinfo(url, NULL, &hints, &result);

    if(error != 0) {                                                //errore nella risoluzione

        fprintf(stderr, "Error resolving URL\n");
        return NULL; 

    }

    if(result == NULL) {                                            //nessun risultato

        fprintf(stderr, "No results found for the URL.\n");
        return NULL;

    }



    //estraggo l'ip (si trova dentro result->ai_addr)
    struct sockaddr *addr_sporca = result->ai_addr;                 //prendo l'indirizzo, ma devo castarlo, se no non posso leggerlo, deve diventare sockaddr_in

    struct sockaddr_in *addr = (struct sockaddr_in *)addr_sporca;   //casto a sockaddr_in, per poter estrarre l'ip

    struct in_addr ip_bin = addr->sin_addr;                         //estraggo l'ip reale in binario



    //converto il binario in stringa
    char buffer[INET_ADDRSTRLEN];                                       //buffer temporaneo per la conversione da binario a stringa

    if(inet_ntop(AF_INET, &ip_bin, buffer, sizeof(buffer)) == NULL) {   //converto da binario a stringa

        fprintf(stderr, "Error converting binary IP to string.\n");
        freeaddrinfo(result);                                           //libero la memoria allocata da getaddrinfo
        return NULL;   

    }

    char *ip_string = malloc(strlen(buffer) + 1);                       //devo allocarlo dinamicamente per forza, se no nella return non viene salvato e la variabile si autodistrugge
    strcpy(ip_string, buffer);                                          //copio il buffer nella stringa dinamica


    printf("Resolved URL %s to IP: %s\n", url, ip_string);              //stampo l'ip risolto

    freeaddrinfo(result);                                               //libero la memoria allocata da getaddrinfo

    return ip_string; 






}


char *reverse_dns(struct in_addr ip_bin){

    char *url = malloc(BUFFER_SIZE); //alloco dinamicamente per poter farla uscire dalla funzione
    char buffer_tmp[BUFFER_SIZE]; //buffer temporaneo per getnameinfo, che con l'allocazione dinamica mi dava errore

    //getnameinfo() vuole una struct sockaddr_in, quindi devo crearla
    struct sockaddr_in ip_addr;
    memset(&ip_addr, 0, sizeof(ip_addr)); //inizializzo a zero

    ip_addr.sin_family = AF_INET; 
    ip_addr.sin_addr = ip_bin; 
    ip_addr.sin_port = 0; //per il reverse dns non serve la porta

    //getnameinfo mi serve per la risoluzione dns inversa
    //devo passare come parametri la mia struct sockaddr_in castata in sockaddr (creare direttamente sockaddr risulta complesso), la su a dimesnsione, il buffer, la sua dimensione
    //poi passo null per la scelta del servizio (non mi interessa avere dei servizi in particolere, http etc...), la dimensione di quel buffer (NULL) e la flag 0 in quanto voglio un comportamneto standard
    //da notare che così anche se non trova nessun url, restituisce l'ip in stringa, così da poter sempre stampare qualcosa, anche quando la risoluzione fallisce
    int error = getnameinfo((struct sockaddr *)&ip_addr, sizeof(ip_addr), buffer_tmp, sizeof(buffer_tmp), NULL, 0, 0);

    if(error != 0) { 
        //fprintf(stderr, "Error resolving IP to URL\n"); 
        return NULL; 
    }
    strcpy(url, buffer_tmp); //trasferisco l'url
    //printf("Resolved IP %s to URL: %s\n", inet_ntoa(ip_bin), url);

    return url;

}


double gettimestamp(){

    struct timespec ts; //struttura per il timestamp
    clock_gettime(CLOCK_MONOTONIC, &ts); //prendo il timestamp corrente, uso CLOCK_MONOTONIC per avere un ts indipendente dall'ora del sistema, meglio di CLOCK_REALTIME

    //voglio il risultato in ms ma ho secondoni e nanosecondi, moltiplico per 1000 i secondi e divido i nanosecondi per 1000000
    double timestamp = ts.tv_sec*1000 + ts.tv_nsec/1000000;

    //printf("Current timestamp: %.2f ms\n", timestamp); //stampo il timestamp per testing (.2f è per avere due decimali)
    return timestamp; //ritorno il timestamp in ms


}





//FUNZIONI IPV6

int check_ipv6(char *ip, struct in6_addr *ip_bin){

    int val = inet_pton(AF_INET6, ip, ip_bin);      //converte l'ip in un formato binario e salva il status code in val

    if(val == 1) {

        //devo salvare l'ip in una variabile (sia come stringa per la stampa, sia come binario per le socket)
        store_ip(ip, NULL, ip_bin, 6);   //salvo l'ip in formato binario, passando la variabile per argomento

        printf("Valid IPv6 address provided: %s\n", ip);

        
    
    }else if(val == 0) {

        //devo risolvere l'url in un ip
        printf("Not valid IPv6 address, resolving URL: %s\n", ip);

        char *resolved_ip = dns_resolver_ipv6(ip);     //risolvo l'url in un ip

        //salvo l'ip
        if(resolved_ip == NULL) {

            fprintf(stderr, "Error resolving URL to IP.\n");
            return 1;

        }

        store_ip(resolved_ip, NULL, ip_bin, 6);                     //salvo l'ip in formato binario, passando la variabile per argomento

    }else {

        fprintf(stderr, "Error in checking IP address.\n");
        return 1;

    }


}

char *dns_resolver_ipv6(char *url){

    struct addrinfo hints; 
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET6;                                      //specifico ipv6
    hints.ai_socktype = SOCK_DGRAM;                                 //specifico socket udp

    struct addrinfo *result;
    int error = getaddrinfo(url, NULL, &hints, &result);

    if(error != 0) {                                                //errore nella risoluzione

        fprintf(stderr, "Error resolving URL\n");
        return NULL; 

    }

    if(result == NULL) {                                            //nessun risultato

        fprintf(stderr, "No results found for the URL.\n");
        return NULL;

    }

    struct sockaddr *addr_sporca = result->ai_addr;                 //il procedimento è lo stesso di ipv4 ma con sockaddr_in6
    struct sockaddr_in6 *addr = (struct sockaddr_in6 *)addr_sporca;   
    struct in6_addr ip_bin = addr->sin6_addr;                         //estraggo l'ip reale in binario


    char buffer[INET6_ADDRSTRLEN];                                       //buffer temporaneo per la conversione da binario a stringa

    if(inet_ntop(AF_INET6, &ip_bin, buffer, sizeof(buffer)) == NULL) {   //converto da binario a stringa

        fprintf(stderr, "Error converting binary IP to string.\n");
        freeaddrinfo(result);                                           //libero la memoria allocata da getaddrinfo
        return NULL;   

    }

    char *ip_string = malloc(strlen(buffer) + 1);                       //devo allocarlo dinamicamente per forza, se no nella return non viene salvato e la variabile si autodistrugge
    strcpy(ip_string, buffer);                                          //copio il buffer nella stringa dinamica


    printf("Resolved URL %s to IP: %s\n", url, ip_string);              //stampo l'ip risolto

    freeaddrinfo(result);                                               //libero la memoria allocata da getaddrinfo

    return ip_string; 



}


char *reverse_dns_ipv6(struct in6_addr ip_bin){

    char *url = malloc(BUFFER_SIZE); //alloco dinamicamente per poter farla uscire dalla funzione
    char buffer_tmp[BUFFER_SIZE]; //buffer temporaneo per getnameinfo, che con l'allocazione dinamica mi dava errore

    //getnameinfo() vuole una struct sockaddr_in, quindi devo crearla
    struct sockaddr_in6 ip_addr;
    memset(&ip_addr, 0, sizeof(ip_addr)); //inizializzo a zero

    ip_addr.sin6_family = AF_INET6; 
    ip_addr.sin6_addr = ip_bin; 
    ip_addr.sin6_port = 0;          //per il reverse dns non serve la porta

    //getnameinfo mi serve per la risoluzione dns inversa
    //devo passare come parametri la mia struct sockaddr_in castata in sockaddr (creare direttamente sockaddr risulta complesso), la su a dimesnsione, il buffer, la sua dimensione
    //poi passo null per la scelta del servizio (non mi interessa avere dei servizi in particolere, http etc...), la dimensione di quel buffer (NULL) e la flag 0 in quanto voglio un comportamneto standard
    //da notare che così anche se non trova nessun url, restituisce l'ip in stringa, così da poter sempre stampare qualcosa, anche quando la risoluzione fallisce
    int error = getnameinfo((struct sockaddr *)&ip_addr, sizeof(ip_addr), buffer_tmp, sizeof(buffer_tmp), NULL, 0, 0);

    if(error != 0) { 
        
        return NULL; 
    }
    strcpy(url, buffer_tmp); //trasferisco l'url
    

    return url;

}