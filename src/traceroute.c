
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
void test_stampa_ip(struct in_addr ip_bin, char *ip_string);



int main(int argc, char *argv[]) {

    //inizio con il controllo dei permessi di root, necessari per la socket raw/icmp
    //controllo quindi l'effective user ID (euid)
    int euid = geteuid();

    if(euid != 0) {
        fprintf(stderr, "Error: This program must be run as root.\n");
        return 1;
    }

    //controllo se l'utente ha fornito il numero di parametri minimi
    if(argc < 2){
        fprintf(stderr, "Too few arguments, please provide an IP address or an URL.\n");
        return 1;
    }


    //controllo se l'utente ha fornito un ip o un url
    if(check_ipv4(argv[1]) == 1) {

        //devo salvare l'ip in una variabile (sia come stringa per la stampa, sia come binario per le socket)
        char *ip_string = argv[1]; //la stringa

        struct in_addr ip_bin; //ora vuota

        store_ip(ip_string, &ip_bin); //salvo l'ip in formato binario, passando la variabile per argomento

        printf("Valid IPv4 address provided: %s\n", argv[1]);

        //test di stampa
        test_stampa_ip(ip_bin, ip_string);
    
    }else if(check_ipv4(argv[1]) == 0) {

        //devo risolvere l'url in un ip
        printf("Not valid IPv4 address, resolving URL: %s\n", argv[1]);
    }else{
        fprintf(stderr, "Error in checking IP address.\n");
        return 1;
    }
    
    


    return 0;
}




void test_stampa_ip(struct in_addr ip_bin, char *ip_string) {

    //test di stampa sia dal formato binario, che da argv[]
    char ip_str[INET_ADDRSTRLEN]; //buffer per la conversione da binario a stringa
    if(inet_ntop(AF_INET, &ip_bin, ip_str, sizeof(ip_str)) == NULL) { //il contrario di inet_pton
        fprintf(stderr, "Error converting binary IP to string.\n");
        return;
    }
    printf("Binary IP: %s\n", ip_str);
    printf("In string format: %s\n", ip_string); //stampo l'ip in formato stringa

}





