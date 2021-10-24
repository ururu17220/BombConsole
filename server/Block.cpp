#include "Block.hpp"
#include "SquareObject.hpp"


SquareObject::req_delete_t Block::run(){
    setSquareMapAttr(x, y, BLOCK_ATTR);
    return false;
}

void Block::drawAA(){
    print("■");
}

Block::~Block(){
    
}