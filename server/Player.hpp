#ifndef __PLAYER_HPP__
#define __PLAYER_HPP__

#include "SquareObject.hpp"
#include "Bomb.hpp"
#include <vector>
#include <unordered_map>

class Bomb;     // Forward declaration

class Player : public SquareObject{
    public:
    // Constructor
    Player(int id_);
    Player(int ix, int iy);

    static Player* find(int id_);
    
    // Destructor
    ~Player();

    enum class Direction{UP, DOWN, RIGHT, LEFT, NONE};

    void moveTo(Player::Direction direction);
    void moveTo(int x_, int y_);
    
    void createBomb();

    int kills;
    static std::vector<Player*> death;
    
    private:
    req_delete_t run() override;
    void drawAA() override;

    // Bomb
    int creatable_num;
    std::vector<Bomb*> created_bombs;

    enum class StateIs{Living, Dying, Died} state;
    timespec state_transition_time;

    static std::unordered_map<int, Player*> id_Player;
};


#endif