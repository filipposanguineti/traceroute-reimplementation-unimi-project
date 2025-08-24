
#include <stdio.h>
#include <unistd.h>             //è necessaria per usare geteuid()
#include <arpa/inet.h>          //necessaria per usare le funzioni di rete
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>         //necessaria per le funzioni di socket
#include <netinet/in.h>         //necessaria per le strutture di rete
#include <netdb.h>              //necessaria per le funzioni di risoluzione DNS
#include <sys/types.h>          //necessaria per estendere i tipi di dato
#include <time.h>               //necessaria per clock_gettime e timespec
#include <sys/time.h>           //necessaria per la struct timeval


#include "utils.h"              
#include "udp.h"                
#include "icmp.h"               
#include "struct.h"
#include "print.h"

#define BUFFER_SIZE 1500        





//dichiarazioni funzioni
void test_stampa_ip(struct in_addr ip_bin, char *ip_string);
int trace(struct in_addr dest);
int trace_ipv6(struct in6_addr dest);



int main(int argc, char *argv[]) {

    //inizio con il controllo dei permessi di root, necessari per la socket raw/icmp
    //controllo quindi l'effective user ID (euid)
    int euid = geteuid();

    if(euid != 0) {

        fprintf(stderr, "Error: This program must be run as root.\n");
        return 1;

    }

    //controllo se l'utente ha fornito il numero di parametri minimi
    if(argc < 2) {

        fprintf(stderr, "Too few arguments, please provide an IP address or an URL.\n");
        return 1;

    }






    if(argc < 3){

        //IPV4

        //controllo se l'utente ha fornito un ip o un url
        struct in_addr ip_bin;                                      //questa è la variabile in cui si salverà l'ip da usare nelle socket
        check_ipv4(argv[1], &ip_bin);                               //chiamo la funzione check_ipv4 per verificare se l'ip è valido o se è un url da risolvere


        //TRACEROUTE

        char buffer[INET_ADDRSTRLEN];                               //buffer per la conversione da binario a stringa
        inet_ntop(AF_INET, &ip_bin, buffer, sizeof(buffer));
        print_intro(buffer, argv[1]);                               //stampo l'intro con l'ip e l'url
        trace(ip_bin);                                              //chiamo la funzione trace con l'ip binario

    
        return 0;
        
        
    }else if(strcmp(argv[2], "-ipv6") == 0){

        //IPV6
        
        struct in6_addr ip_bin6;
        int resolved = check_ipv6(argv[1], &ip_bin6);               //chiamo la funzione check_ipv6 per verificare se l'ip è valido o se è un url da risolvere

        if(resolved == 1){
            printf("IPV6 address not disponible\n");
            return 0;
        }

        char buffer[INET6_ADDRSTRLEN];                           
        inet_ntop(AF_INET6, &ip_bin6, buffer, sizeof(buffer));
        print_intro(buffer, argv[1]);                           
        trace_ipv6(ip_bin6); 
        return 0;                                         

    }


    
}




void test_stampa_ip(struct in_addr ip_bin, char *ip_string) {

    //test di stampa sia dal formato binario, che da argv[]
    char ip_str[INET_ADDRSTRLEN];                                       //buffer per la conversione da binario a stringa

    if(inet_ntop(AF_INET, &ip_bin, ip_str, sizeof(ip_str)) == NULL) {   //il contrario di inet_pton
        fprintf(stderr, "Error converting binary IP to string.\n");
        return;

    }

    printf("Binary IP: %s\n", ip_str);
    printf("In string format: %s\n", ip_string);                        

}



int trace(struct in_addr dest){

    //funzione principale che gestisce tutto il traceroute, e il doppio ciclo for

    //VARIABILI

    int udp_sd;                                     //creo la socket udp
    int icmp_sd;                                    //creo la socket raw icmp
    int ttl = 1;                                    //il ttl parte da 1
    int probe = 0;                                  //indice del probe, parte da 0 così da calcoare la porta di destinazione e distinguere i prbe
    int send_port;                                  //porta calcolata all'invio del pacchetto udp
    int max_ttl = 30;                               //traceroute si spinge fino a 30 hop, quindi non vado oltre
    char reply[BUFFER_SIZE];                        //buffer per la risposta icmp
    struct in_addr reply_addr;                      //la struct per l'indirizzo da cui ho ricevuto risposta
    char reply_addr_string[INET_ADDRSTRLEN];        //buffer per la conversione da binario a string
    int icmp_error_code;                            //codice di errore ricevuto in icmp
    int reply_port;                                 //porta del probe che ha ricevuto la risposta, per ritrovare il probe giusto
    double ts0;                                     //timestamp del probe
    double ts1;                                     //timestamp della risposta
    int hop_number = 0;                             //numero di ho che mi serirà nella stampa di ognuno di essi


    udp_sd = create_socket_udp();
    icmp_sd = create_socket_raw_icmp();

    for(ttl = 1; ttl <= max_ttl; ttl++){

        ttl_increment(udp_sd, ttl, 4);                                                      //setto il ttl nella socket udp
        
        informations array_probe[3] = {0};                                                  //array di struct per salvare i dati dei probe

        for(probe = 0; probe < 3; probe++){

            
            ts0 = gettimestamp();                                                           //prendo il timestamp prima di inviare il probe
            send_probe(udp_sd, dest, ttl, probe, &send_port);                               //invio il probe

            
            
            informations info = {0};                                                        //la struct per i dati la dichiaro qui così da essere azzerata automaticamente ad ogni invio                                               

            //metto un ciclo infinito per la select, finché non ha trattato tutte le risposte in coda
            while(1){

                
                //volendo usare la select, devo gestire il timeout e settare alcune variabili
                struct timeval timeout;
                timeout.tv_sec = 3;                                                         //metto un timeout di 3 secondi
                timeout.tv_usec = 0;                                                        //microsecondi a 0

                fd_set read_fds;                                                            //creo il set di file descriptor che la select monitora
                FD_ZERO(&read_fds);                                                         //inizializzo il set a zero
                FD_SET(icmp_sd, &read_fds);                                                 //aggiungo la socket icmp al set

                int result = select(icmp_sd + 1, &read_fds, NULL, NULL, &timeout); 

                //setto di default la struct info come se non avesse trovato risultato
                info.rtt = -1;
                strncpy(info.ip_string, "*", INET_ADDRSTRLEN);                              //setto *
                strncpy(info.url, "*", BUFFER_SIZE);                                        //setto * per l'url

                if(result < 0) {

                    fprintf(stderr, "Error in select.\n");
                    break;

                }else if(result == 0) {
                    
                    break;
                }

                //se la select ha trovato qualcosa da leggere, leggo dalla socket icmp
                memset(reply, 0, BUFFER_SIZE);                                              //azzero il buffer di risposta
                int rec = receive_icmp(icmp_sd, reply, NULL, NULL, 4);                      //ricevo un pacchetto icmp

                if(rec < 0) {

                    fprintf(stderr, "Error receiving ICMP packet.\n");
                    break;
                    
                }

                //estraggo i dati dal pacchetto ricevuto
                extract_rec_data(reply, &reply_addr, reply_addr_string, &icmp_error_code, &reply_port);

                char *url_translated = reverse_dns(reply_addr);                             //faccio il reverse dns dell'indirizzo di risposta

                

                if(send_port == reply_port) {

                    //se la porta del probe corrisponde a quella della risposta, ho trovato il probe giusto

                    ts1 = gettimestamp();                                                   //prendo il timestamp della risposta

                    //aggiungo alla struct l'rtt
                    info.rtt = ts1 - ts0;                                                   //rtt caloclato come ts di arrivo - ts di partenza

                    //se sono arrivato fin qui significa che ho ricevuto una risposta valida
                    //setto la struct info con i dati ricevuto 
                    strncpy(info.ip_string, reply_addr_string, INET_ADDRSTRLEN);
                    strncpy(info.url, url_translated, BUFFER_SIZE);                         


                    //controllo se sono arrivato alla fine (codice errore 3)
                    if(icmp_error_code == 3) {

                        print_final(info);                                                  //stampo il risultato finale
                        close_socket_udp(udp_sd);                                           //chiudo la socket udp
                        close_socket_icmp(icmp_sd);                                         //chiudo la socket icmp
                        return 0;                                                           //esco dalla funzione (non solo dal ciclo)

                    }


                    break;                                                                  //esco dal ciclo dei probe in attesa di essere processati dalla select

                }


            }

            
            //qui devo popolare l'array di struct informations per salvare il probe corrente
            array_probe[probe].rtt = info.rtt;                                              //salvo l'rtt
            strncpy(array_probe[probe].ip_string, info.ip_string, INET_ADDRSTRLEN);         //salvo nell'array l'ip del prbe corrente
            strncpy(array_probe[probe].url, info.url, BUFFER_SIZE);                         //salvo l'url del probe corrente

        }

        //ora devo stampare i risultati dei probe per questo ttl (stampo tre probe alla volta)
        hop_number++;                                                                       //incremento il numero di hop
        print_line(array_probe, hop_number);                                                //stampo i risultati dei probe dopo ogni tre probe (ad ogni ttl)

        
    }

    return 0;

}




int trace_ipv6(struct in6_addr dest){

    //VARIABILI

    int udp_sd;                                     //creo la socket udp
    int icmp_sd;                                    //creo la socket raw icmp
    int ttl = 1;                                    //il ttl parte da 1
    int probe = 0;                                  //indice del probe, parte da 0 così da calcoare la porta di destinazione e distinguere i prbe
    int send_port;                                  //porta calcolata all'invio del pacchetto udp
    int max_ttl = 30;                               //traceroute si spinge fino a 30 hop, quindi non vado oltre
    char reply[BUFFER_SIZE];                        //buffer per la risposta icmp
    struct in6_addr reply_addr;                     //la struct per l'indirizzo da cui ho ricevuto risposta
    char reply_addr_string[INET6_ADDRSTRLEN];       //buffer per la conversione da binario a string
    int icmp_error_code;                            //codice di errore ricevuto in icmp
    int reply_port;                                 //porta del probe che ha ricevuto la risposta, per ritrovare il probe giusto
    double ts0;                                     //timestamp del probe
    double ts1;                                     //timestamp della risposta
    int hop_number = 0;                             //numero di ho che mi serirà nella stampa di ognuno di essi

    udp_sd = create_socket_udp_ipv6();
    icmp_sd = create_socket_raw_icmp_ipv6();

    for(ttl = 1; ttl <= max_ttl; ttl++){

        ttl_increment(udp_sd, ttl, 6);                                                      //setto il ttl nella socket udp
        
        informations_ipv6 array_probe[3] = {0};                                             //array di struct per salvare i dati dei probe

        for(probe = 0; probe < 3; probe++){

            
            ts0 = gettimestamp();                                                           //prendo il timestamp prima di inviare il probe
            send_probe_ipv6(udp_sd, dest, ttl, probe, &send_port);                          

            
            //la struct per i dati la dichiaro qui così da essere azzerata automaticamente ad ogni invio
            informations_ipv6 info = {0};                                         

            //metto un ciclo infinito per la select, finché non ha trattato tutte le risposte in coda
            while(1){

                
                //volendo usare la select, devo gestire il timeout e settare alcune variabili
                struct timeval timeout;
                timeout.tv_sec = 3;                                                         //metto un timeout di 3 secondi
                timeout.tv_usec = 0;                                                        //microsecondi a 0

                fd_set read_fds;                                                            //creo il set di file descriptor che la select monitora
                FD_ZERO(&read_fds);                                                         //inizializzo il set a zero
                FD_SET(icmp_sd, &read_fds);                                                 //aggiungo la socket icmp al set

                int result = select(icmp_sd + 1, &read_fds, NULL, NULL, &timeout); 

                //setto di default la struct info come se non avesse trovato risultato
                info.rtt = -1;
                strncpy(info.ip_string, "*", INET6_ADDRSTRLEN);      
                strncpy(info.url, "*", BUFFER_SIZE);                

                if(result < 0) {

                    fprintf(stderr, "Error in select.\n");
                    break;

                }else if(result == 0) {
                    
                    break;
                }

                //se la select ha trovato qualcosa da leggere, leggo dalla socket icmp
                memset(reply, 0, BUFFER_SIZE);                                              //azzero il buffer di risposta
                int rec = receive_icmp(icmp_sd, reply, &reply_addr, reply_addr_string, 6);      

                if(rec < 0) {

                    fprintf(stderr, "Error receiving ICMP packet.\n");
                    break;
                    
                }

                //estraggo i dati dal pacchetto ricevuto
                extract_rec_data_ipv6(reply, &icmp_error_code, &reply_port);
                char *url_translated = reverse_dns_ipv6(reply_addr);                        //faccio il reverse dns dell'indirizzo di risposta

                

                if(send_port == reply_port) {

                    //se la porta del probe corrisponde a quella della risposta, ho trovato il probe giusto

                    ts1 = gettimestamp();                                                   //prendo il timestamp della risposta

                    //aggiungo alla struct l'rtt
                    info.rtt = ts1 - ts0;                                                   //rtt caloclato come ts di arrivo - ts di partenza

                    //se sono arrivato fin qui significa che ho ricevuto una risposta valida
                    //setto la struct info con i dati ricevuto 
                    strncpy(info.ip_string, reply_addr_string, INET6_ADDRSTRLEN);
                    strncpy(info.url, url_translated, BUFFER_SIZE);     

                    
                    //controllo se sono arrivato alla fine (codice errore 4)
                    if(icmp_error_code == 4) {

                        print_final_ipv6(info);                                             //stampo il risultato finale
                        close_socket_udp(udp_sd);                                           //chiudo la socket udp
                        close_socket_icmp(icmp_sd);                                         //chiudo la socket icmp
                        return 0;                                                           //esco dalla funzione (non solo dal ciclo)

                    }


                    break;                                                                  //esco dal ciclo dei probe in attesa di essere processati dalla select

                }


            }

            
            //qui devo creare un array di struct informations per salvare il probe corrente
            array_probe[probe].rtt = info.rtt;                                              //salvo l'rtt
            strncpy(array_probe[probe].ip_string, info.ip_string, INET6_ADDRSTRLEN);        //salvo nell'array l'ip del prbe corrente
            strncpy(array_probe[probe].url, info.url, BUFFER_SIZE);                         //salvo l'url del probe corrente

        }

        //ora devo stampare i risultati dei probe per questo ttl
        hop_number++;                                                                       //incremento il numero di hop
        print_line_ipv6(array_probe, hop_number);                                           //stampo i risultati dei probe dopo ogni tre probe (ad ogni ttl)

        
    }
    
    return 0;





}





