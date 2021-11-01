#include "SquareObject.hpp"
#include <ncurses.h>
#include <locale.h>
#include <time.h>

SquareObject::SquareObject()
    :x(0), y(0)
{
    object_list.push_back(this);
}

SquareObject::SquareObject(int x_, int y_)
    :x(x_), y(y_)
{
    object_list.push_back(this);
}

SquareObject::~SquareObject(){

}

void SquareObject::setMapSize(int map_size_x, int map_size_y){
    square_map = std::vector<std::vector<uint8_t>>(map_size_x, std::vector<uint8_t>(map_size_y));
    square_map_p = std::vector<std::vector<uint8_t>>(map_size_x, std::vector<uint8_t>(map_size_y));
    MAP_SIZE_X = map_size_x;
    MAP_SIZE_Y = map_size_y;
}

void SquareObject::getMapSize(int *map_size_x, int *map_size_y){
    *map_size_x = MAP_SIZE_X;
    *map_size_y = MAP_SIZE_Y;
}

void SquareObject::runAllObjects(){
    clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
    for(int i = 0; i < MAP_SIZE_X; i++){
        for(int j = 0; j < MAP_SIZE_Y; j++){
            square_map_p[i][j] = square_map[i][j];
            square_map[i][j] = 0x00;
        }
    }
    for(auto itr = object_list.begin(); itr != object_list.end(); ++itr){
        SquareObject *sqobj = *itr;
        req_delete_t req = sqobj->run();
        if(req){
            // delete request
            itr = object_list.erase(itr);
            --itr;
            delete sqobj;
        }
        else
            sqobj->drawAA();
    }

}

void SquareObject::runAllObjects(std::vector<uint8_t>::iterator &square_map_cylinder){
    runAllObjects();
    for(int j = 0; j < MAP_SIZE_Y; j++)
        for(int i = 0; i < MAP_SIZE_X; i++)
            *(square_map_cylinder++) = square_map[i][j]; 
}

void SquareObject::print(const char *aa){
    mvprintw(y, x*MAP_SQ_SIZE_X, aa);
}

uint8_t SquareObject::setSquareMapAttr(int map_x, int map_y, uint8_t map_attr){
    if(map_x >= MAP_SIZE_X || map_y >= MAP_SIZE_Y)
        return 0xff;    // error
    return square_map[map_x][map_y] |= map_attr;
}

uint8_t SquareObject::getSquareMapAttr(int map_x, int map_y){
    if(map_x >= MAP_SIZE_X || map_y >= MAP_SIZE_Y)
        return 0x00;    // error
    return square_map_p[map_x][map_y];
}

bool SquareObject::isElapsed(timespec *timepoint){
    return (
        current_time.tv_sec > timepoint->tv_sec || 
        (
        current_time.tv_sec == timepoint->tv_sec && 
        current_time.tv_nsec >= timepoint->tv_nsec
        )
        );
}