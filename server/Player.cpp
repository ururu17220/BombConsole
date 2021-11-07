#include "Player.hpp"
#include "SquareObject.hpp"
#include <time.h>

Player::Player(int id_){
    id_Player[id_] = this;
    living.insert(this);
    creatable_num = 4;
    kills = 0;
    state = StateIs::Living;
}

Player::Player(int ix, int iy) : SquareObject(ix, iy){
    creatable_num = 4;
    kills = 0;
    state = StateIs::Living;
}

Player* Player::find(int id_){
    return id_Player[id_];
}

SquareObject::req_delete_t Player::run(){
    uint8_t map_attr = getSquareMapAttr(x, y);
    switch (state){
    case StateIs::Living:       
        if(map_attr & (BLOCK_ATTR | FIRE_ATTR)){
            // Death
            state = StateIs::Dying;
            state_transition_time.tv_sec = current_time.tv_sec + 1;
            state_transition_time.tv_nsec = current_time.tv_nsec;
            death.push_back(this);
            living.erase(this);
        }
        else
            setSquareMapAttr(x, y, PLAYER_LIVING_ATTR);
        break;

    case StateIs::Dying:
        setSquareMapAttr(x, y, PLAYER_DYING_ATTR);
        if(isElapsed(&state_transition_time))
            state = StateIs::Died;
        break;
    case StateIs::Died:
        break;
    }
    return false;
}

void Player::drawAA(){
    switch (state){
    case StateIs::Living:
        print("人");
        break;
    case StateIs::Dying:
        print("死");
        break;
    case StateIs::Died:
        break;
    }
}

Player::~Player(){

}

void Player::moveTo(Player::Direction direction){
    if(state != StateIs::Living)
        return;
        
    int dest_x = x, dest_y = y;
    switch(direction){
        case Direction::UP:     dest_y--;    break;
        case Direction::DOWN:   dest_y++;    break;
        case Direction::LEFT:   dest_x--;    break;
        case Direction::RIGHT:  dest_x++;    break;
    }
    // Check whether the Player can move to destination.
    if(!(getSquareMapAttr(dest_x, dest_y) & (BLOCK_ATTR | BOMB_ATTR))){
        // Player can move to destination, if there are no Block or Bomb.
        x = dest_x;
        y = dest_y;
    }
}

void Player::moveTo(int x_, int y_){
    x = x_;
    y = y_;
}

void Player::createBomb(){
    if(state == StateIs::Living && !(getSquareMapAttr(x, y) & (BOMB_ATTR | BLOCK_ATTR)))
        created_bombs.push_back(new Bomb(this, x, y, 4));
}

uint8_t Player::getState(){
    return (uint8_t)state;
}