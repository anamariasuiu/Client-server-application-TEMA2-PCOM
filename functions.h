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

using namespace std;

struct tcp_message {
    char ip[16];
    char port[10];
    char topic[51];
    char type[10];
    char payload[1501];
    tcp_message() {}
};

struct message {
    char id[10];
    char topic[50];
    char type[20];
    int sf;
};
struct topic {
    char name[50];
    int sf;
};
struct client {
    char id[10];
    bool active;
    int fd;
    vector <tcp_message> unsentMessages;
    vector <topic> topics;
};
