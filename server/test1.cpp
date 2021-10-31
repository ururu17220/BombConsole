#include <ncurses.h>
#include <locale.h>
#include <vector>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <string>
#include <string.h>

#include "ServerClient.hpp"
#include "SquareObject.hpp"
#include "Block.hpp"
#include "Player.hpp"
#include "color_def.h"

#define PORT    8000

std::mutex mtx;

std::list<SquareObject*> SquareObject::object_list;
uint8_t SquareObject::square_map[MAP_SIZE_X][MAP_SIZE_Y] = {0};
uint8_t SquareObject::square_map_p[MAP_SIZE_X][MAP_SIZE_Y] = {0};
timespec SquareObject::current_time;
std::unordered_map<int, Player*> Player::id_Player;
std::vector<Player*> Player::death;

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

    // send coordinate
    uint8_t xy[2] = {1, 3};
    for(auto itr = server.clients.begin(); itr != server.clients.end(); itr++){
        itr->second->send(xy, sizeof(xy)/sizeof(xy[0]));
        Player::death.push_back(Player::find(itr->second->getSocket()));    //for test
        xy[0] = xy[1]++;
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