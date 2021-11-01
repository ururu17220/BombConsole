#include <ncurses.h>
#include <locale.h>
#include <vector>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <string>
#include <string.h>
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

    int num = 0;

    Server server(PORT);    
    server.onConnect = [](Client *c){
        new Player(c->getSocket());
    };

    server.onReceive = [](Client *c, uint8_t *receive_data, int len){
        Player *p = Player::find(c->getSocket());
        strcpy(p->name, (const char *)receive_data); 
        printw("%s\n", receive_data);
    };

    // waiting Players
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
    SquareObject::setMapSize(7, 7);
    int map_size_x, map_size_y;
    SquareObject::getMapSize(&map_size_x, &map_size_y);
    make_map();

    // send coordinate
    uint8_t send_data[4];
    uint8_t x_init = 1;
    uint8_t y_init = 1;
    for(auto itr = server.clients.begin(); itr != server.clients.end(); itr++){
        send_data[0] = x_init;
        send_data[1] = y_init;
        send_data[2] = map_size_x;
        send_data[3] = map_size_y;

        x_init += 4;
        if(x_init >= map_size_x){
            x_init = 1;
            y_init += 4;
        }
        itr->second->send(send_data, sizeof(send_data)/sizeof(send_data[0]));
        Player::death.push_back(Player::find(itr->second->getSocket()));    //for test
    }

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

    endwin();
    waitingPlayers.detach();
    return 0;
}