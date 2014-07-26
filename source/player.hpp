#ifndef LAMCO_PLAYER_HPP
#define LAMCO_PLAYER_HPP

#include "basic.hpp"
#include "map.hpp"
#include <iostream>

using namespace std;

class Player
{
public:
    void init(Position pos, istream& is);

    void step(const Map& map);
    void reset();

    Position position() const;

private:
    Position _startPosition;
    Position _position;
    Direction _direction;
};

#endif