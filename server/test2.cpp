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


Player *gm;
void gamemaster(int key){
    switch (key){
        case KEY_UP:    gm->moveTo(Player::Direction::UP);      break;
        case KEY_DOWN:  gm->moveTo(Player::Direction::DOWN);    break;
        case KEY_LEFT:  gm->moveTo(Player::Direction::LEFT);    break;
        case KEY_RIGHT: gm->moveTo(Player::Direction::RIGHT);   break;
        case ' ':       gm->createBomb();                       break;
        default:    break;
    }
}

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
    // test
    make_map();

    gm = new Player(3, 5);


    Server server(PORT);    
    server.onConnect = [](Client *c){
        Player *p = new Player(c->getSocket());
        p->moveTo(3, 3);
    };

    server.onReceive = [](Client *c, uint8_t *receive_data, int len){
        Player *p = Player::find(c->getSocket());
        strcpy(p->name, (const char *)receive_data); 
    };

    // wait client
    server.waitClients();
    uint8_t send_data[MAP_SIZE_X*MAP_SIZE_Y + 1];   // Map data and Player's death or living

    while(1){
        clear();
        mtx.lock();
        SquareObject::runAllObjects(send_data+1);

        // send data 
        for(auto itr = server.clients.begin(); itr != server.clients.end(); itr++){ 
            Client *c = itr->second;
            Player *p = Player::find(c->getSocket());
            send_data[0] = p->getState();
            c->send(send_data, sizeof(send_data) / sizeof(send_data[0]));
        }

        mtx.unlock();
        refresh();
        int key = getch();
        if(key=='q')break;
        gamemaster(key);
    }

    // game end
    uint8_t game_end_command = (uint8_t)'\n';
    server.broadcast(&game_end_command, 1);

    endwin();

    return 0;
}