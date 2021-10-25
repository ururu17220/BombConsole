#ifndef __SERVER_CLIENT_HPP__
#define __SERVER_CLIENT_HPP__

#define CLIENT_BUFFER_SIZE  32
#define SEND_RCV_TIMEOUT    1       //[s]

#include <thread>
#include <unordered_map>

class Server;

class Client{
    public:
    friend Server;

    // Destructor
    ~Client();
    int send(const uint8_t *send_data, int len);
    void (*onReceive)(Client *c, uint8_t *receive_data, int len);

    int getSocket();

    private:
    // Constructor
    Client(int ss_, void (*onReceive_)(Client *c, uint8_t *receive_data, int len));

    // socket
    int ss;

    // thread
    std::thread *t;
    void task();
    bool isEnd;

    // Buffer
    uint8_t buf[CLIENT_BUFFER_SIZE];
};

//

class Server{
    public:
    // Constructor
    Server(uint16_t port);

    // Destructor
    ~Server();

    void (*onConnect)(Client *c);
    void (*onReceive)(Client *c, uint8_t *receive_data, int len);

    Client* waitClients();


    void broadcast(uint8_t *send_data, int len);

    std::unordered_map<int, Client*> clients;

    private:
    int s;
};


#endif