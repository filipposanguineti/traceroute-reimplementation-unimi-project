
#include <stdio.h>
#include <unistd.h> //è necessaria per usare getruid()
#include <arpa/inet.h> //necessaria per usare le funzioni di rete




//dichiarazioni funzioni
int check_ipv4(char *ip);
void store_ip(char *ip, struct in_addr *ip_bin);



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
    
    }else if(check_ipv4(argv[1]) == 0) {

        //devo risolvere l'url in un ip
        printf("Not valid IPv4 address, resolving URL: %s\n", argv[1]);
    }else{
        fprintf(stderr, "Error in checking IP address.\n");
        return 1;
    }
    
    


    return 0;
}





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