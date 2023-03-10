#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <algorithm>
#include "helpers.h"
#include "functions.h"

using namespace std;
void usage(char *file) {
    fprintf(stderr, "Usage: %s server_port\n", file);
    exit(0);
}
void find_payload(char *buffer, tcp_message *tcp_m) {
    unsigned int type = buffer[50];
    DIE((type > 3 || type < 0), "Didn't write a valid type");

    switch (type) {
        case 0: {
            strcpy(tcp_m->type, "INT");
            int32_t number = htonl(*(uint32_t *)(buffer + 52));
            if (buffer[51] == 1)
                number = (-1) * number;
            sprintf(tcp_m->payload, "%d", (int)number);
            break;
        }
        case 1: {
            strcpy(tcp_m->type, "SHORT_REAL");
            float num = ntohs(*(uint16_t *)(buffer + 51));
            num /= (float)(100 * 1.0);
            sprintf(tcp_m->payload, "%.2f", num);
            break;
        }
        case 2: {
            strcpy(tcp_m->type, "FLOAT");
            int sign = *(buffer + 51);
            float numb = ntohl(*(uint32_t*)(buffer + 52));
            int e = (int)*(buffer + 56);
            numb /= pow(10, e);
            if (*(buffer + 51) == 1)
                numb *= -1;
            sprintf(tcp_m->payload, "%lf", numb);
            break;
        }
        default:
            strcpy(tcp_m->type, "STRING");
            strcpy(tcp_m->payload, (buffer + 51));
            break;
    }
}
int main(int argc, char* argv[]) {
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    int tcp_sock, udp_sock, new_sock, portno;
    int ret, i, n, flag = 1;

    char buffer[BUFLEN];
    struct sockaddr_in tcp_addr, udp_addr, cli_addr;
	socklen_t clilen = sizeof(cli_addr);

    fd_set read_fds; //multimea de citire folosita de select
	fd_set tmp_fds; //multime folosita temporar
	int fdmax;  //valoarea maxima fd din multimea read_fds

	tcp_message tcp_m;
	vector <client> clients;

    FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	if (argc < 2) { 
		usage(argv[0]);
	}

    // deschid un socket pentru udp
	udp_sock = socket(PF_INET, SOCK_DGRAM, 0);
	DIE(udp_sock < 0, "Unable to create UDP socket");

	// deschid un socket pentru tcp
	tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
	DIE(tcp_sock < 0, "Unable to create TCP socket");

	portno = atoi(argv[1]);
	DIE(portno == 0, "Port number is not valid");

	memset((char *) &tcp_addr, 0, sizeof(tcp_addr));
	tcp_addr.sin_family = AF_INET;
	tcp_addr.sin_port = htons(portno);
	tcp_addr.sin_addr.s_addr = INADDR_ANY;

    memset((char *) &udp_addr, 0, sizeof(udp_addr));
	udp_addr.sin_family = AF_INET;
	udp_addr.sin_port = htons(portno);
	udp_addr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(tcp_sock, (struct sockaddr *)&tcp_addr, sizeof(struct sockaddr));
    DIE(ret < 0, "Unable to bind TCP socket");

    ret = bind(udp_sock, (struct sockaddr *)&udp_addr, sizeof(struct sockaddr));
    DIE(ret < 0, "Unable to bind UDP socket");

    ret = listen(tcp_sock, MAX_CLIENTS);
    DIE(ret < 0, "Unable to listen to any TCP client");

	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(tcp_sock, &read_fds);
	FD_SET(udp_sock, &read_fds);
	FD_SET(0, &read_fds);

	fdmax = max(tcp_sock, udp_sock);

	while (1) {
		tmp_fds = read_fds;
		memset(buffer, 0, BUFLEN);
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "Could not select socket");

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == STDIN_FILENO) {
					memset(buffer, 0, BUFLEN);
					fgets(buffer, BUFLEN - 1, stdin);

					if (strncmp(buffer, "exit", 4) == 0) {
						for(int i = 1; i <= fdmax; i++) {
							if((i != tcp_sock) &&(i != udp_sock)){
							if (FD_ISSET(i, &read_fds)) {
								strncpy(tcp_m.payload, "exit", 4);
								n = send(i, &tcp_m, sizeof(tcp_message),0);
								DIE(n < 0, "Sending failed");
								close(i);
							}
							}
						close(tcp_sock);
						close(udp_sock);
						exit(0);
						}
						break;
					}
				} else if (i == tcp_sock) {
					new_sock = accept(tcp_sock, (struct sockaddr *) &cli_addr, &clilen);
					DIE(new_sock < 0, "Could not find connection");

					FD_SET(new_sock, &read_fds);

					// dezactivez algoritmul lui Nagle
					int result = setsockopt(new_sock, IPPROTO_TCP, TCP_NODELAY, (char*) &flag, sizeof(int));
					DIE(result < 0, "Setsockopt failed");

					ret = recv(new_sock, buffer, BUFLEN - 1, 0);
					DIE(ret < 0, "Could not receive ID from client");

					bool connected = false;

					// daca exista deja un client conectat cu acelasi id
					for(unsigned int j = 0; j < clients.size(); j++) {
						if(clients[j].active == true && strcmp(clients[j].id, buffer) == 0) {
							printf("Client %s already connected.\n", buffer);
							connected = true;
							close(new_sock);
						}

						// daca a mai fost conectat un client cu acelasi id
						if(clients[j].active == false && strcmp(clients[j].id, buffer) == 0) {
							printf("New client %s connected from %s:%hu.\n", clients[j].id,
                               	inet_ntoa(cli_addr.sin_addr), htons(cli_addr.sin_port));
							connected = true;
							if (!clients[j].unsentMessages.empty())
							{
								for (auto message : clients[j].unsentMessages)
								{
									n = send(new_sock, (char *)&message, sizeof(tcp_message), 0);
									DIE(n < 0, "send");
								}
								clients[j].unsentMessages.clear();
							}
							clients[j].active = true;
						}
					}
					// daca nu exista un client conectat cu acelasi id
					// cream unul nou
					if(!connected) {
						fdmax = max(fdmax, new_sock);

						client new_client;
						memset(&new_client, 0, sizeof(client));
						new_client.fd = new_sock;
						new_client.active = true;
						strcpy(new_client.id, buffer);

						clients.push_back(new_client);
						printf("New client %s connected from %s:%hu.\n", new_client.id,
                               inet_ntoa(cli_addr.sin_addr), htons(cli_addr.sin_port));
					}
				} else if (i == udp_sock) {
					// primesc mesaj de la client udp
					memset(buffer, 0, BUFLEN);
					memset(&tcp_m, 0, sizeof(tcp_message));
					n = recvfrom(i, buffer, sizeof(buffer), 0, (struct sockaddr*) &cli_addr ,&clilen);

					DIE(n < 0, "Could not receive UDP message");

					strcpy(tcp_m.ip, inet_ntoa(cli_addr.sin_addr));
					sprintf(tcp_m.port, "%u", (unsigned int)ntohs(cli_addr.sin_port));
					strcpy(tcp_m.topic, buffer);
					find_payload(buffer, &tcp_m);

					// cautam in vectorul de clienti pentru fiecare topic al sau
					for(unsigned int j = 0; j < clients.size(); j++) {
						for(unsigned k = 0; k < clients[j].topics.size(); k++) {
	
							if(clients[j].active == false && strcmp(clients[j].topics[k].name, tcp_m.topic) == 0 
								&& clients[j].topics[k].sf == 1){
								clients[j].unsentMessages.push_back(tcp_m);
							} 
							if(clients[j].active == true && strcmp(clients[j].topics[k].name, tcp_m.topic) == 0)  {
								n = send(clients[j].fd, (char *)&tcp_m, sizeof(tcp_message), 0);
								DIE(n < 0, "Could not send the message to TCP client");
							}
						}
					}

				} else {
					message msg;
					memset(&msg, 0, sizeof(message));

                    n = recv(i, &msg, sizeof(message), 0);
                    DIE(n < 0, "Could not receive data from clients");

					// conexiunea s-a inchis
					if(n == 0) {
                        FD_CLR(i, &read_fds);
                        close(i);
					} 

					// cautam indexul-ul clientului cu id ul msg.id
					int index;
					for(unsigned int j = 0; j < clients.size(); j++) {
							if(strcmp(clients[j].id ,msg.id) == 0) {
								index = j;
							}
					}
					if (strncmp(msg.type, "exit", 4) == 0) {
						printf("Client %s disconnected.\n", msg.id);
						clients[index].active = false;

					} else if (strcmp(msg.type, "subscribe") == 0) {
						bool topic_exist = false;
						for(auto it = clients[index].topics.begin(); it < clients[index].topics.end();) {
							if(strcmp((*it).name, msg.topic) == 0) {
								topic_exist = true;
							} 
							else {
								it++;
							}
						}
						if(!topic_exist){
							topic new_topic;
							strcpy(new_topic.name, msg.topic);
							new_topic.sf = msg.sf;
							clients[index].topics.push_back(new_topic);
						}
					} else if (strcmp(msg.type, "unsubscribe") == 0) {
						for(auto it = clients[index].topics.begin(); it < clients[index].topics.end(); it++) {
							if(strcmp((*it).name, msg.topic) == 0) {
								clients[index].topics.erase(it);
							}
						}
					}

				}

			}

		}

	}
	
    close(tcp_sock);
    close(udp_sock);

    return 0;
}