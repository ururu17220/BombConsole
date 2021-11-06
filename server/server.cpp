#include <ncurses.h>
#include <locale.h>
#include <vector>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <string.h>
#include <string>
#include <math.h>

#include "ServerClient.hpp"
#include "SquareObject.hpp"
#include "Block.hpp"
#include "Player.hpp"
#include "color_def.h"

#define PORT    8000

std::mutex mtx;

std::list<SquareObject*> SquareObject::object_list;
std::vector<std::vector<uint8_t>> SquareObject::square_map;
std::vector<std::vector<uint8_t>> SquareObject::square_map_p;
int SquareObject::MAP_SIZE_X;
int SquareObject::MAP_SIZE_Y;
timespec SquareObject::current_time;
std::unordered_map<int, Player*> Player::id_Player;
std::vector<Player*> Player::death;
std::unordered_set<Player*> Player::living;

void make_map(){
    int i,j;
    int map_size_x, map_size_y;
    SquareObject::getMapSize(&map_size_x, &map_size_y);
    for(i = 0; i < map_size_x; i++)
        for(j = 0; j < map_size_y; j++)
            if(
                (i == 0 || j == 0 || i == map_size_x-1 || j == map_size_y-1)
                ||
                (i%2 + j%2 == 0)
            )
               new Block(i, j);
}

int main(void){
    // ncurses initialize
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    halfdelay(1);
    curs_set(0);
    start_color();
    color_init();

    // Server initialize
    Server server(PORT);    

    server.onConnect = [](Client *c){
        new Player(c->getSocket());
    };

    server.onReceive = [](Client *c, uint8_t *receive_data, int len){
        Player *p = Player::find(c->getSocket());
        strcpy(p->name, (const char *)receive_data); 
        printw("%s\n", receive_data);
    };

    // wait clients
    bool waitend = false;
    std::thread waitingPlayers([&waitend, &server](){
        while(!waitend)
            server.waitClients();
    });

    while(1){
        int key = getch();
        if(key == 'g' && server.clients.size() != 0)
            break;
        else if(key == 'q'){
            endwin();
            waitingPlayers.detach();
            return 0;
        }
    }
    waitend = true;


    // make map
    int map_size = ceil((double)server.clients.size()/4.0)*4 + 3;
    SquareObject::setMapSize(map_size, map_size);
    make_map();


    // send initial coordinate
    uint8_t send_data_coordinate[4];
    uint8_t x_init = 1;
    uint8_t y_init = 1;
    send_data_coordinate[2] = map_size;
    send_data_coordinate[3] = map_size;
    for(auto itr = server.clients.begin(); itr != server.clients.end(); itr++){
        send_data_coordinate[0] = x_init;
        send_data_coordinate[1] = y_init;
        Player::find(itr->second->getSocket())->moveTo(x_init, y_init);
        itr->second->send(send_data_coordinate, sizeof(send_data_coordinate)/sizeof(send_data_coordinate[0]));
        x_init += 4;
        if(x_init >= map_size){
            x_init = 1;
            y_init += 4;
        }
    }


    // game start
    mtx.lock();
    server.onConnect = [](Client *c){
        // ignore
    };
    server.onReceive = [](Client *c, uint8_t *receive_data, int len){
        Player *p = Player::find(c->getSocket());
        int x = receive_data[0];
        int y = receive_data[1];
        int bomb = receive_data[2];
        p->moveTo(x, y);
        if(bomb)
            p->createBomb();
    };
    mtx.unlock();


    std::vector<uint8_t> send_data(map_size * map_size + 1);   // Map data and Player's death or living

    // game loop
    while(Player::living.size() > 1){
        clear();

        mtx.lock();
        auto send_data_itr = send_data.begin();
        send_data_itr += 1;
        SquareObject::runAllObjects(send_data_itr);
        mtx.unlock();

        // send data 
        for(auto itr = server.clients.begin(); itr != server.clients.end(); itr++){ 
            Client *c = itr->second;
            Player *p = Player::find(c->getSocket());
            send_data[0] = p->getState();
            c->send(send_data.data(), send_data.size());
        }

        refresh();
        int key = getch();
        if(key=='q')break;  // Force end
    }

    // game end
    uint8_t game_end_command = (uint8_t)'\n';
    server.broadcast(&game_end_command, 1);

    if(Player::living.size() == 1)
        Player::death.push_back(*Player::living.begin());


    // send ranking
    std::string ranking;
    for(int rank = 0; rank < Player::death.size(); rank++){
        Player *p = Player::death[rank];
        ranking += std::to_string(rank+1);
        ranking += "位 : ";
        ranking += p->name;
        ranking += '\n';
    }

    server.broadcast((const uint8_t *)ranking.c_str(), ranking.length());
    
    waitingPlayers.detach();

    endwin();

    printf("RANKING\n%s\n", ranking.c_str());

    return 0;
}