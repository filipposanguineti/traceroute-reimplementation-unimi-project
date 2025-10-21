# Traceroute Reimplementation in C

**Progetto universitario realizzato per il corso di Reti di Calcolatori.**  
L’obiettivo è stato quello di re-implementare il programma **traceroute** in linguaggio **C**.

---

## Descrizione

Il programma ricostruisce il percorso verso un host di destinazione inviando pacchetti UDP con valori di TTL crescenti.  
Per ogni pacchetto inviato, viene ricevuta una risposta ICMP che indica se il router attraversato è intermedio o finale.
Il processo viene ripetuto fino al raggiungimento del router finale.
Il progetto supporta sia IPv4 che IPv6.

---

## Tecnologie utilizzate

- **Linguaggio:** C
- **Socket UDP** per l'invio dei probe UDP
- **Socket raw ICMP** per la ricezione e gestione dei pacchetti ICMP  

---

## Esecuzione

**Compilazione:** dalla directory principale eseguire "make"
**Uso IPv4:** sudo ./bin/traceroute 'indirizzo ip o url'
**Uso IPv6:** sudo ./bin/traceroute 'indirizzo ip o url' -ipv6
