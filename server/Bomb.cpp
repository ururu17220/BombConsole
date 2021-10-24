#include "Bomb.hpp"
#include "Fire.hpp"
#include "SquareObject.hpp"
#include "Player.hpp"
#include "color_def.h"

#include <ncurses.h>
#include <time.h>

Bomb::Bomb(Player *creator_, int x_, int y_, int power_) :
    SquareObject(x_, y_),
    creator(creator_),
    power(power_)
{
    explosion_time.tv_sec = current_time.tv_sec + BOMB_LIFETIME;
    explosion_time.tv_nsec = current_time.tv_nsec;
}


Bomb::~Bomb(){}

SquareObject::req_delete_t Bomb::run(){
    if(isElapsed(&explosion_time)){
        explosion();
        return true;
    }
    setSquareMapAttr(x, y, BOMB_ATTR);

    if(getSquareMapAttr(x, y) & FIRE_ATTR){
        //Ignition
        explosion_time.tv_sec -= BOMB_LIFETIME;
    }

    return false;
}

void Bomb::drawAA(){
    attron(COLOR_PAIR(BOMB_COLOR));
    mvprintw(y, x*MAP_SQ_SIZE_X, "爆");
    attron(COLOR_PAIR(BOMB_FUSE_COLOR) | A_BLINK);
    mvprintw(y, x*MAP_SQ_SIZE_X+2, "*");
    attroff(COLOR_PAIR(BOMB_FUSE_COLOR) | A_BLINK);
}

void Bomb::explosion(){
    int x0, x1, y0, y1;

    // Block and Bomb stops fire
    uint8_t attr;
    for(x0 = x-1; x0 > x-power; x0--){
        attr = getSquareMapAttr(x0, y);
        if(attr & BLOCK_ATTR){
            x0++;
            break;
        }
        else if(attr & BOMB_ATTR)
            break;
    }

    for(x1 = x+1; x1 < x+power; x1++){
        attr = getSquareMapAttr(x1, y);
        if(attr & BLOCK_ATTR){
            x1--;
            break;
        }
        else if(attr & BOMB_ATTR)
            break;
    }
    
    for(y0 = y-1; y0 > y-power; y0--){
        attr = getSquareMapAttr(x, y0);
        if(attr & BLOCK_ATTR){
            y0++;
            break;
        }
        else if(attr & BOMB_ATTR)
            break;
    }
    
    for(y1 = y+1; y1 < y+power; y1++){
        attr = getSquareMapAttr(x, y1);
        if(attr & BLOCK_ATTR){
            y1--;
            break;
        }
        else if(attr & BOMB_ATTR)
            break;
    }
    
    new Fire(creator, x, y, x0, x1, y0, y1);
}