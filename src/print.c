#include <stdio.h>
#include <unistd.h> //è necessaria per usare geteuid()
#include <arpa/inet.h> //necessaria per usare le funzioni di rete
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h> //necessaria per le funzioni di socket
#include <netinet/in.h> //necessaria per le strutture di rete
#include <netdb.h> //necessaria per le funzioni di risoluzione DNS
#include <sys/types.h> //necessaria per estendere i tipi di dato
#include <time.h> //necessaria per clock_gettime e timespec
#include <sys/time.h> //necessaria per la struct timeval


#include "utils.h" //includo il file header per le dichiarazioni delle funzioni
#include "udp.h" //includo il file header per le dichiarazioni delle funzioni udp
#include "icmp.h" //includo il file header per le dichiarazioni delle funzioni icmp
#include "traceroute.h" //includo il file header per le dichiarazioni delle funzioni di stampa
#include "struct.h"

#define BUFFER_SIZE 1500 // Definisco una costante per la dimensione del buffer




void print_line(informations *array){

    for(int i = 0; i<3; i++){

        

        if(array->rtt == -1){

            printf("%s (%s) * ms", array[i].ip_string, array[i].url);
        
        } else{
            printf("%s (%s) %.2f ms", array[i].ip_string, array[i].url, array[i].rtt);
        }
    }

    printf("\n");
    return;
    
}

void print_intro(char *ip, char *url){

    printf("Traceroute to %s (%s)", ip, url);
}