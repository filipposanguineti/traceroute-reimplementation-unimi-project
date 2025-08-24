#include <stdio.h>
#include <unistd.h>                 //è necessaria per usare geteuid()
#include <arpa/inet.h>              //necessaria per usare le funzioni di rete
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>             //necessaria per le funzioni di socket
#include <netinet/in.h>             //necessaria per le strutture di rete
#include <netdb.h>                  //necessaria per le funzioni di risoluzione DNS
#include <sys/types.h>              //necessaria per estendere i tipi di dato
#include <time.h>                   //necessaria per clock_gettime e timespec
#include <sys/time.h>               //necessaria per la struct timeval


#include "utils.h"                  
#include "udp.h"                    
#include "icmp.h"                   
#include "traceroute.h"             
#include "struct.h"
#include "print.h"                  

#define BUFFER_SIZE 1500            // Definisco una costante per la dimensione del buffer




void print_line(informations *array, int hop_number){

    //ricevo un array di tre elementi, ciclo e stampo i tre risultati

    print_numbers(hop_number);                                                                      //stampo il numero dell'ho corrente

    for(int i = 0; i<3; i++){

        

        if(array->rtt == -1){

            printf("[%s (%s) * ms]\t\t", array[i].ip_string, array[i].url);
        
        } else{
            printf("[%s (%s) %.2f ms]\t\t", array[i].ip_string, array[i].url, array[i].rtt);
        }
    }

    printf("\n"); 
    return;
    
}

void print_intro(char *ip, char *url){

    printf("Traceroute to %s (%s)\n", ip, url);

    return;
}

void print_numbers(int hop_number){

    
    printf("Hop %d: ", hop_number);

    return;
}

void print_final(informations end){

    printf("Reached destination: %s (%s) %.2f ms\n", end.ip_string, end.url, end.rtt);

    return;
}

void print_final_ipv6(informations_ipv6 end){

    printf("Reached destination: %s (%s) %.2f ms\n", end.ip_string, end.url, end.rtt);

    return;

    
}


void print_line_ipv6(informations_ipv6 *array, int hop_number){

    //ricevo un array di tre elementi, ciclo e stampo i tre risultati

    print_numbers(hop_number);      

    for(int i = 0; i<3; i++){

        

        if(array->rtt == -1){

            printf("[%s (%s) * ms]\t\t", array[i].ip_string, array[i].url);
        
        } else{
            printf("[%s (%s) %.2f ms]\t\t", array[i].ip_string, array[i].url, array[i].rtt);
        }
    }

    printf("\n"); 
    return;
    
}