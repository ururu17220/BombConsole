#ifndef __FIRE_HPP__
#define __FIRE_HPP__

#include "SquareObject.hpp"
#include "Bomb.hpp"
#include "Player.hpp"
#include <time.h>

#define FIRE_LIFETIME 1

class Player;   // Forward declaration

class Fire : public SquareObject{
    public:
    // Constructor
    Fire(Player *creator_, int x_, int y_, int left_fire_x_, int right_fire_x_, int down_fire_y, int up_fire_y);

    // Destrucotr
    virtual ~Fire();

    protected:
    virtual req_delete_t run() override;
    virtual void drawAA() override;

    const int left_fire_x;
    const int right_fire_x;
    const int up_fire_y;
    const int down_fire_y;

    timespec fire_time;

    Player *creator;
};

#endif