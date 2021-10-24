#ifndef __BOMB_HPP__
#define __BOMB_HPP__

#include "SquareObject.hpp"
#include "Player.hpp"
#include "Fire.hpp"
#include <time.h>

#define BOMB_LIFETIME   2

class Player;   // Forward declaratoin

class Bomb : public SquareObject{
    public:
    // Constructor
    Bomb(Player *creator_, int x_, int y_, int power_);
    
    // Destructor
    virtual ~Bomb();

    protected:
    virtual req_delete_t run() override;
    virtual void drawAA() override;

    virtual void explosion();

    Player *creator;

    // explosion time 
    timespec explosion_time;

    // Range of fire
    int power;
    
};


#endif