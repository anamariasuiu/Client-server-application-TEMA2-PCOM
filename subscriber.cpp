#include <iostream>
#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include "helpers.h"
#include "functions.h"

using namespace std;

void usage(char *file) {
    fprintf(stderr, "Usage: %s <ID_CLIENT> <IP_Server> <Port_Server>\n", file);
    exit(0);
}
int main(int argc, char* argv[]) {
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    int new_sock, portno;
    int ret, i, n, flag = 1;

    char buffer[BUFLEN];
    struct sockaddr_in serv_addr;

    fd_set read_fds; //multimea de citire folosita de select
	fd_set tmp_fds; //multime folosita temporar

    FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);


	if (argc < 4) { 
		usage(argv[0]);
	}

    //deschid un socket pentru tcp
	new_sock = socket(AF_INET, SOCK_STREAM, 0);
	DIE(new_sock < 0, "Unable to create UDP socket");

	portno = atoi(argv[3]);
	DIE(portno == 0, "Port number is not valid");

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "Could not set server address");

	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(new_sock, &read_fds);

    ret = connect(new_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "Could not connect the address to the socket");

	n = send(new_sock, argv[1], strlen(argv[1]), 0);
	DIE(n < 0, "Could not send the client ID");

	// dezactivez algoritmul lui Nagle
	int result = setsockopt(new_sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
	DIE(result < 0, "Setsockopt failed");

	while (1) {
		tmp_fds = read_fds; 
		
		ret = select(new_sock + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "Could not select");

		if (FD_ISSET(STDIN_FILENO, &tmp_fds)) { 
			message msg;
			memset(&msg, 0, sizeof(message));
			strcpy(msg.id, argv[1]);
			memset(buffer, 0, BUFLEN);
			fgets(buffer, BUFLEN - 1, stdin);

			if (strncmp(buffer, "exit", 4) == 0) {
				strncpy(msg.type, buffer, 4);

				n = send(new_sock, &msg, sizeof(message), 0);
				DIE(n < 0, "Could not send exit error");
				return 0;
			} else if (strncmp(buffer, "subscribe", 9) == 0) {
				char *subscribe = strtok(buffer, " ");
				char *topic = strtok(NULL, " ");
				DIE(topic == NULL, "No topic typed");

				char *sf = strtok(NULL, " ");
				DIE(sf == NULL ,"No sf typed");

				int Sf = atoi(sf);
				DIE(Sf != 0 && Sf != 1, "Sf typed incorrecly");

				strcpy(msg.topic, topic);
				strcpy(msg.type, subscribe);
				msg.sf = Sf;

				// trimit mesaj la server
                n = send(new_sock, &msg, sizeof(message), 0);
			    DIE(n < 0, "Could not send message to the server");
				printf("Subscribed to topic\n");


			} else if (strncmp(buffer, "unsubscribe", 11) == 0) {
				char *unsubscribe = strtok(buffer, " ");
				char *topic =  strtok(NULL, " \n");
				DIE(topic == NULL, "No topic typed");

				strcpy(msg.topic, topic);
				strcpy(msg.type, unsubscribe);

				n = send(new_sock, &msg, sizeof(message), 0);
			    DIE(n < 0, "Could not send message to the server");
				printf("Unsubscribed from topic\n");
				
			} 
			else {
				printf("%s\n", "Please enter a valid command");
				exit(0);
			}
		}
		// primesc mesaj de la server
		if (FD_ISSET(new_sock, &tmp_fds)) {
			tcp_message tcp_msg;
			memset(&tcp_msg, 0, sizeof(tcp_message));

			n = recv(new_sock, &tcp_msg, sizeof(tcp_message), 0);
			DIE(n < 0, "Could not receive a message from the server");

			if (n == 0) {
                close(new_sock);
				return 0;  
            }
			if(strcmp(tcp_msg.payload, "exit") == 0) {
				break;
			}
			cout << tcp_msg.ip << ":" << tcp_msg.port << " - " << tcp_msg.topic 
				<< " - " << tcp_msg.type << " - " << tcp_msg.payload <<endl;
		}
		

	}
	close(new_sock);
    return 0;
}