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

#define PORT    8000

std::mutex mtx;

std::list<SquareObject*> SquareObject::object_list;
uint8_t SquareObject::square_map[MAP_SIZE_X][MAP_SIZE_Y] = {0};
uint8_t SquareObject::square_map_p[MAP_SIZE_X][MAP_SIZE_Y] = {0};
timespec SquareObject::current_time;
std::unordered_map<int, Player*> Player::id_Player;

void make_map(){
    int i,j;
    for(i = 0; i < MAP_SIZE_X; i++)
        for(j = 0; j < MAP_SIZE_Y; j++)
            if(
                (i == 0 || j == 0 || i == MAP_SIZE_X-1 || j == MAP_SIZE_Y-1)
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
    
    make_map();

    // Server Initialize
    Server server(PORT);
    server.onConnect = [](Client *c){
        new Player(c->getSocket());
    };
    server.onReceive = [](Client *c, uint8_t *receive_data, int len){
        Player *p = Player::find(c->getSocket());
    };


    int key;
    // waiting Players
    while(1){
        clear();
        server.waitClients();
        refresh();
        key = getch();
        if(key == '\n')
            break;
    }


    while(1){
        clear();
        mtx.lock();
        SquareObject::runAllObjects();
        mtx.unlock();
        refresh();
        int key;
        //read(ss, &key, sizeof(key));
        key = getch();
        if(key=='q')break;
    }

    endwin();

    return 0;
}