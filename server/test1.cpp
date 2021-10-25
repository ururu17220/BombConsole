#include <ncurses.h>
#include <locale.h>
#include <vector>
#include <thread>
#include <mutex>
#include <unistd.h>

#include "ServerClient.hpp"
#include "SquareObject.hpp"
#include "Block.hpp"
#include "Player.hpp"
#include "color_def.h"

#define NOT_DRAW

#define PORT    8000

std::mutex mtx;

std::list<SquareObject*> SquareObject::object_list;
uint8_t SquareObject::square_map[MAP_SIZE_X][MAP_SIZE_Y] = {0};
uint8_t SquareObject::square_map_p[MAP_SIZE_X][MAP_SIZE_Y] = {0};
timespec SquareObject::current_time;
std::unordered_map<int, Player*> Player::id_Player;

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
        // for test
        Player::death.push_back(p); 
    };


    // waiting Players
    while(num++ > 3){
        server.waitClients();
    }

    // send Players coordinate
    uint8_t xy[2] = {1, 1};
    for(auto itr = server.clients.begin(); itr != server.clients.end(); itr++){
        itr->second->send(xy, sizeof(xy)/sizeof(xy[0]));
        xy[0] = xy[1]++;
    }

    return 0;
}