In subscriber.cpp:

In acest fisier am implementat clientul TCP. Am creat un socket, m-am conectat cu ajutorul lui la server
si am trimis serverului id-ul primit ca prim argument. Pentru fiecare comanda primita de la tastura am
luat cate un caz separat: exit, subscribe, unsubscribe si niciuna din acestea. Pentru exit trimit mesaj
la server de tipul structurii tcp_message pentru a instiinta ca vreau sa ma deconectez. Pentru subscribe
si unsubscribe despart sirul primit de la tastatura in tokeni pentru a trimite serverului prin intermediul
structurii message numele comenzii, numele topicul si valoarea lui sf. In cazul in care am primit mesaj
de la server. Daca am primit exit, oprescu programul, daca nu inseamna ca sunt datele de la topicul la care
m-am abonat si le afisez.

In server.cpp:
In acest fisierul este implementarea serverului. Aici creez doi socketi:pentru clientul tcp si pentru clientul
udp. Pentru ambele socketuri fac bind la server pentru a putea asculta pe acestia. Socketul serverului(tcp_sock)
asculta in cazul in care un client se conecteaza. 
Am luat cazurile in care serverul primeste de la stdin comanda exit si  inchid toate conexiunele cu toti clientii.
Cazul 2 cand se primeste activitate pe serverul inactiv , creez socket pentru el si il adauga in multimea de
socketi. Primesc in buffer id-ul clientului. Dupa aceea verific urmatoarele pentru fiecare client din vectorul de
clienti: daca exista deja un client conectat cu acest id, afisez mesaj si inchid conexiunea cu el; daca clinetul 
esti inactiv si are acest id, ii transmit mesajele(la care s-a abonat cu sf 1)neprimite cat timp a fost inactiv, 
daca exista, iar apoi le sterg din vector si il trec activ; daca nu exista client conectat cu acest id creez unul
nou adaugand la lista de clienti de tipul structurii clinent.
Cazul 3 cand serverul primeste activitate de la clientul udp, adica datele topicurile si facem conversie payloadului
in functia find_payload pentru a-i afla valoarea. Cautam in vectorul de clienti pentru fiecare topic pe care il are
daca este topicul transmis de clientul udp. Daca este si clientul este inactiv, cu sf-ul 1 atunci punem datele 
topicului in vectorul unsentMessages. Daca este si clientul este activ, ii transmitem acum datele topicului.
Cazul 4 este cand serverul primeste mesaj de la client tcp. Daca comanda sa e exit atunci il deconectez trecandu-l
inactiv. Daca este subscribe, verific daca topicul nu exista in lista de topicuri a clientului curent si il adaug.
Daca este unsubscribe caut topicul in lista de topicuri si il sterg.

Probleme intalnite: Am testat pe checker tema si avand aceeasi implementare am inchis si am redeschis programul si
aveam rezultate diferite pe teste fata de cele anterioare. Acelasi lucru il patesc si daca rulez manual. De exemplu
cateodata imi deconecteaza clientul daca dau exit, cateodata nu si imi intra intr-o bucla infinita la server. Nu am 
inteles comportamentul acesta si a fost un impediment pentru mine in realizarea completa a temei.