#ifndef LAMCO_MAP_HPP
#define LAMCO_MAP_HPP

#include "basic.hpp"
#include <iostream>
#include <vector>

using namespace std;

class Map
{
public:
    void init(istream& is);

    char get(Position pos) const;
    void set(Position pos, char ch);

    int width() const;
    int height() const;

private:
    void validate() const;

    int _width;
    int _height;
    vector<char> _data;
};

#endif