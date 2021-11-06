#include "ServerClient.hpp"

#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

Client::Client(int ss_, void (**onReceive_)(Client *c, uint8_t *receive_data, int len)):
    ss(ss_),
    onReceive(onReceive_),
    isEnd(false)
{
    t = new std::thread(&Client::task, this);
}

Client::~Client(){
    isEnd = true;
    t->join();
    close(ss);
}

int Client::send(const uint8_t *send_data, int len){
    return write(ss, send_data, len);
}

void Client::task(){
    while(!isEnd){
        int len = read(ss, buf, sizeof(buf) / sizeof(buf[0]));
        if(len > 0){
            (*onReceive)(this, buf, len);
        }
    }
}

int Client::getSocket(){
    return ss;
}

//

Server::Server(uint16_t port){
    s = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);   

    // To avoid "Address already in use"
    int yes = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes)) < 0)
    {
        perror("ERROR on setsockopt (REUSEADDR)");
        exit(1);
    }

    if(bind(s, (struct sockaddr *)&addr, sizeof(addr))){
        perror("ERROR on bind");
        exit(1);
    }
    listen(s, 5);
}

Client* Server::waitClients(){
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    int ss = accept(s, (struct sockaddr *)&client_addr, &len);
    
    // Set time out
    struct timeval tv;
    tv.tv_sec = SEND_RCV_TIMEOUT;
    tv.tv_usec = 0;
    if(setsockopt(ss, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv))){
        perror("ERROR on setsockopt (RCVTIMEO)");
        exit(1);
    }

    Client *c = new Client(ss, &onReceive);
    onConnect(c);
    clients[ss] = c;
    return c;
}

Server::~Server(){
    for(auto itr = clients.begin(); itr != clients.end(); itr++){
        Client *c = itr->second;
        delete c;
    }
    close(s);
}

void Server::broadcast(const uint8_t *send_data, int len){
    for(auto itr = clients.begin(); itr != clients.end(); itr++){
        Client *c = itr->second;
        c->send(send_data, len);
    }
}