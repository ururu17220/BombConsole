#include <vector>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#include "ServerClient.hpp"
#include "SquareObject.hpp"
#include "Block.hpp"
#include "Player.hpp"

#include "curses_conf.h"
#ifndef NO_CURSES
#include <ncurses.h>
#include <locale.h>
#else
#include <termios.h>
#include <fcntl.h>

template <typename ... Args>
void printw(const char *format, Args const & ... args){
    printf(format, args ...);
}

int getch(void){
    std::this_thread::sleep_for(std::chrono::microseconds(100000));
    //usleep(100000);
    struct termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if(ch != EOF){
        return ch;
    }
    return 0;
}

#endif

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

std::vector<uint8_t> send_data;   // Map data and Player's death or living

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
    #ifndef NO_CURSES
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
    #endif 

    // Server initialize
    mtx.lock();
    Server server(PORT);    

    server.onConnect = [](Client *c){
        mtx.lock();
        //printw("connected id=%d\n", c->getSocket());
        new Player(c->getSocket());
        mtx.unlock();
        return true;
    };

    server.onReceive = [](Client *c, uint8_t *receive_data, int len){
        mtx.lock();
        printw("name =");
        Player *p = Player::find(c->getSocket());
        strcpy(p->name, (const char *)receive_data); 
        printw(" %s\n", receive_data);
        mtx.unlock();
    };
    mtx.unlock();

    // wait clients
    printw("waiting for the players...\n");
    printw("Press 'g' : Game Start ,  Press 'q' : Force End\n\n");
    bool waitend = false;
    std::thread waitingPlayers([&waitend, &server](){
        while(!waitend)
            server.waitClients();
    });

    while(1){
        int key = getch();
        if(key == 'g' && server.clients.size() > 1)
            break;
        else if(key == 'q'){
            waitingPlayers.detach();

            #ifndef NO_CURSES
            endwin();
            #endif 
            return 0;
        }

    }
    waitingPlayers.detach();
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
    printw("\nGame Start!\n");
    mtx.lock();
    server.onConnect = [](Client *c){
        // reject
        return false;
    };
    server.onReceive = [](Client *c, uint8_t *receive_data, int len){
        int x, y;
        Player *p = Player::find(c->getSocket());
        int map_size_x, map_size_y;
        SquareObject::getMapSize(&map_size_x, &map_size_y);
        // make data
        std::vector<uint8_t> send_data_(send_data);
        send_data_[0] = p->getState();
        // modify (remove PLAYER_ATTR)
        p->getXY(&x, &y);
        send_data_[1 + map_size_y*y + x] &= ~PLAYER_ATTR;
        c->send(send_data_.data(), send_data_.size());

        x = receive_data[0];
        y = receive_data[1];
        int bomb = receive_data[2];
        p->moveTo(x, y);
        if(bomb)
            p->createBomb();

    };
    mtx.unlock();


    send_data = std::vector<uint8_t>(map_size * map_size + 1);

    // game loop
    while(Player::living.size() > 1){
        #ifndef NO_CURSES
        clear();
        #endif 

        mtx.lock();
        auto send_data_itr = send_data.begin();
        send_data_itr += 1;
        SquareObject::runAllObjects(send_data_itr);
        mtx.unlock();

        #ifndef NO_CURSES
        refresh();
        #endif
        int key = getch();
        if(key=='q')break;  // Force end
    }

    // game end
    mtx.lock();
    server.onReceive = [](Client *c, uint8_t *receive_data, int len){
        // ignore
    };
    mtx.unlock();
    printw("Game End!\n\n");
    sleep(1);
    uint8_t game_end_command = (uint8_t)'\n';
    server.broadcast(&game_end_command, 1);

    while(Player::living.size()){
        auto itr = Player::living.begin();
        Player::death.push_back(*itr);
        Player::living.erase(itr);
    }


    // send ranking
    std::string ranking;
    for(int rank = 1; rank <= Player::death.size(); rank++){
        Player *p = Player::death[Player::death.size() - rank];
        ranking += std::to_string(rank);
        ranking += "位 : ";
        ranking += p->name;
        ranking += '\n';
    }

    server.broadcast((const uint8_t *)ranking.c_str(), ranking.length());

    #ifndef NO_CURSES
    endwin();
    #endif 

    printf("RANKING\n%s\n", ranking.c_str());

    return 0;
}