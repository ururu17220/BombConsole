#ifndef __PLAYER_HPP__
#define __PLAYER_HPP__

#include "SquareObject.hpp"
#include "Bomb.hpp"
#include <vector>

class Bomb;     // Forward declaration

class Player : public SquareObject{
    public:
    // Constructor
    Player();
    Player(int ix, int iy);
    
    // Destructor
    ~Player();

    enum class Direction{UP, DOWN, RIGHT, LEFT, NONE};

    void moveTo(Player::Direction direction);

    void createBomb();

    private:
    req_delete_t run() override;
    void drawAA() override;

    // Bomb
    int creatable_num;
    std::vector<Bomb*> created_bombs;

    enum class StateIs{Living, Dying, Died} state;
    timespec state_transition_time;
};


#endif