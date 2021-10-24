#ifndef __BLOCK_HPP__
#define __BLOCK_HPP__

#include "SquareObject.hpp"

class Block : public SquareObject{
    public:
    // Constructor
    using SquareObject::SquareObject;

    // Destructor
    ~Block();

    protected:
    req_delete_t run() override;
    void drawAA() override;



};

#endif