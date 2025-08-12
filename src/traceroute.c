
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
#include "struct.h"
#include "print.h"

#define BUFFER_SIZE 1500 // Definisco una costante per la dimensione del buffer





//dichiarazioni funzioni
void test_stampa_ip(struct in_addr ip_bin, char *ip_string);
int trace(struct in_addr dest);



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
    struct in_addr ip_bin; //questa è la variabile in cui si salverà l'ip da usare nelle socket

    if(check_ipv4(argv[1]) == 1) {

        //devo salvare l'ip in una variabile (sia come stringa per la stampa, sia come binario per le socket)
        char *ip_string = argv[1]; //la stringa
        store_ip(ip_string, &ip_bin); //salvo l'ip in formato binario, passando la variabile per argomento

        printf("Valid IPv4 address provided: %s\n", argv[1]);

        
    
    }else if(check_ipv4(argv[1]) == 0) {

        //devo risolvere l'url in un ip
        printf("Not valid IPv4 address, resolving URL: %s\n", argv[1]);

        char *resolved_ip = dns_resolver_ipv4(argv[1]); //risolvo l'url in un ip
        //salvo l'ip
        if(resolved_ip == NULL) {
            fprintf(stderr, "Error resolving URL to IP.\n");
            return 1;
        }
        store_ip(resolved_ip, &ip_bin); //salvo l'ip in formato binario, passando la variabile per argomento


        //alla fine libero la memoria di resolved_ip
        free(resolved_ip);

    }else{
        fprintf(stderr, "Error in checking IP address.\n");
        return 1;
    }

    // int sd = create_socket_udp(); //creo la socket udp
    // ttl_increment(sd, 22); 
    // stampa_ttl_test(sd); //stampo il ttl della socket
    // int probe_port; //variabile per la porta del probe
    // send_probe(sd, ip_bin, 22, 0, &probe_port); //invio un probe all'ip con ttl 12 e probe index 0

    // int sd2 = create_socket_raw_icmp(); //creo la socket raw icmp
    // char buffer[BUFFER_SIZE]; //creo un buffer per ricevere i pacchetti icmp
    // receive_icmp(sd2, buffer); //ricevo un pacchetto icmp
    // //estraggo i dati dal pacchetto ricevuto
    // struct in_addr addr_extract; //variabile per l'indirizzo estratto
    // char addr_string[INET_ADDRSTRLEN]; //buffer per la conversione da binario a stringa
    // int error;
    // int port_rec;
    // extract_rec_data(buffer, &addr_extract, addr_string, &error, &port_rec); //estraggo i dati dal pacchetto ricevuto
    // printf("Extracted IP: %s, Error Code: %d, Port: %d\n", addr_string, error, port_rec); //stampo i dati estratti
    // reverse_dns(addr_extract); //faccio il reverse dns dell'indirizzo estratto
    // gettimestamp(); //ottengo il timestamp corrente

    // close_socket_udp(sd); //chiudo la socket udp
    // close_socket_icmp(sd2); //chiudo la socket raw icmp
    char buffer[INET_ADDRSTRLEN]; //buffer per la conversione da binario a stringa
    inet_ntop(AF_INET, &ip_bin, buffer, sizeof(buffer));
    print_intro(buffer, argv[1]); //stampo l'intro con l'ip e l'url
    trace(ip_bin); //chiamo la funzione trace con l'ip binario

    
    


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



int trace(struct in_addr dest){

    //funzione principale che gestisce tutto il traceroute, e il doppio ciclo for

    //inizio dichiarando le variabili necessarie
    int udp_sd; //creo la socket udp
    int icmp_sd; //creo la socket raw icmp
    int ttl = 1; //il ttl parte da 1
    int probe = 0; //indice del probe, parte da 0 così da calcoare la porta di destinazione e distinguere i prbe
    int send_port; //porta calcolata all'invio del pacchetto udp
    int max_ttl = 30; //traceroute si spinge fino a 30 hop, quindi non vado oltre
    char reply[BUFFER_SIZE]; //buffer per la risposta icmp
    struct in_addr reply_addr; //la struct per l'indirizzo da cui ho ricevuto risposta
    char reply_addr_string[INET_ADDRSTRLEN]; //buffer per la conversione da binario a string
    int icmp_error_code; //codice di errore ricevuto in icmp
    int reply_port; //porta del probe che ha ricevuto la risposta, per ritrovare il probe giusto
    double ts0; //timestamp del probe
    double ts1; //timestamp della risposta
    int hop_number = 0; //numero di ho che mi serirà nella stampa di ognuno di essi


    udp_sd = create_socket_udp();
    icmp_sd = create_socket_raw_icmp();

    for(ttl = 1; ttl <= max_ttl; ttl++){

        ttl_increment(udp_sd, ttl); //setto il ttl nella socket udp
        
        informations array_probe[3] = {0}; //array di struct per salvare i dati dei probe

        for(probe = 0; probe < 3; probe++){

            //printf("Sending probe %d with TTL %d\n", probe, ttl);
            ts0 = gettimestamp(); //prendo il timestamp prima di inviare il probe
            send_probe(udp_sd, dest, ttl, probe, &send_port); //invio il probe

            
            //la struct per i dati la dichiaro qui così da essere azzerata automaticamente
            informations info;

            //metto un ciclo infinito per la select, finché non ha trattato tutte le risposte in coda
            while(1){

                

                //volendo usare la select, devo gestire il timeout e settare alcune variabili
                struct timeval timeout;
                timeout.tv_sec = 3; //metto un timeout di 3 secondi
                timeout.tv_usec = 0; //microsecondi a 0

                fd_set read_fds; //creo il set di file descriptor che la select monitora
                FD_ZERO(&read_fds); //inizializzo il set a zero
                FD_SET(icmp_sd, &read_fds); //aggiungo la socket icmp al set

                int result = select(icmp_sd + 1, &read_fds, NULL, NULL, &timeout); //chiamo la select

                //setto di default la struct info come se non avesse trovato risultato
                info.rtt = -1;
                strncpy(info.ip_string, "*", INET_ADDRSTRLEN); //setto *
                strncpy(info.url, "*", BUFFER_SIZE); //setto * per l'url

                if(result < 0){
                    fprintf(stderr, "Error in select.\n");

                    break;

                }else if(result == 0) {
                    //se il timeout è scaduto, esco dal ciclo
                    
                    //printf("Timeout reached.\n");
                    break;
                }

                //se la select ha trovato qualcosa da leggere, leggo dalla socket icmp
                memset(reply, 0, BUFFER_SIZE); //azzero il buffer di risposta
                int rec = receive_icmp(icmp_sd, reply); //ricevo un pacchetto icmp
                if(rec < 0) {
                    fprintf(stderr, "Error receiving ICMP packet.\n");

                    
                    break;
                }

                //estraggo i dati dal pacchetto ricevuto
                extract_rec_data(reply, &reply_addr, reply_addr_string, &icmp_error_code, &reply_port);
                char *url_translated = reverse_dns(reply_addr); //faccio il reverse dns dell'indirizzo di risposta

                //se sono arrivato fin qui significa che ho ricevuto una risposta valida
                //setto la struct info con i dati ricevuto (tranne rtt che calcolo solo se riesco ad accoppiare invio e ricezione)
                strncpy(info.ip_string, reply_addr_string, INET_ADDRSTRLEN);
                strncpy(info.url, url_translated, BUFFER_SIZE); //salvo l'indirizzo della risposta
                

                if(send_port == reply_port) {
                    //se la porta del probe corrisponde a quella della risposta, ho trovato il probe giusto
                    ts1 = gettimestamp(); //prendo il timestamp della risposta
                    //printf("TTL: %d, Probe: %d, IP: %s, Port: %d, RTT: %.3f ms\n", ttl, probe, reply_addr_string, reply_port, (ts1 - ts0) * 1000);
                    


                    //aggiungo alla struct l'rtt
                    
                    info.rtt = ts1 - ts0; //rtt in ms

                    
                


                    if(icmp_error_code == 3) {
                        //se il codice di errore è 3, ho raggiunto l'host di destinazione
                        //printf("Reached destination: %s\n", reply_addr_string);
                        print_final(info); //stampo il risultato finale
                        close_socket_udp(udp_sd); //chiudo la socket udp
                        close_socket_icmp(icmp_sd); //chiudo la socket icmp
                        return 0; //esco dalla funzione
                    }


                    break; //esco dal ciclo dei probe
                }

                

                


            }

            //qui devo creare un array di struct information per salvare il probe corrente
            array_probe[probe].rtt = info.rtt; //salvo l'rtt
            strncpy(array_probe[probe].ip_string, info.ip_string, INET_ADDRSTRLEN); //salvo nell'array l'ip del prbe corrente
            strncpy(array_probe[probe].url, info.url, BUFFER_SIZE); //salvo l'url del probe corrente




        }

        //ora devo stampare i risultati dei probe per questo ttl
        hop_number++; //incremento il numero di hop
        print_line(array_probe, hop_number); //stampo i risultati dei probe dopo ogni tre probe (ad ogni ttl)

    }

    return 0;

}





