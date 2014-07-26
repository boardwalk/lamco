#include "map.hpp"
#include <cassert>
#include <string>

void Map::init(istream& is)
{
    _width = 0;
    _height = 0;
    _data.clear();

    if(!is)
    {
        throw runtime_error("bad input stream");
    }

    while(is)
    {
        string str;
        getline(is, str);

        if(str.empty())
        {
            continue;
        }
        else if(_width == 0)
        {
            _width = str.size();
        }
        else if(_width != (int)str.size())
        {
            throw runtime_error("mismatched map width");
        }

        _data.insert(_data.end(), str.begin(), str.end());
        _height++;
    }

#ifndef NDEBUG
    validate();
#endif
}

char Map::get(Position pos) const
{
    assert(pos.x >= 0 && pos.x < _width && pos.y >= 0 && pos.y < _height);
    return _data[pos.x + pos.y * _width];
}

void Map::set(Position pos, char ch)
{
    assert(pos.x >= 0 && pos.x < _width && pos.y >= 0 && pos.y < _height);
    _data[pos.x + pos.y * _width] = ch;
}

int Map::width() const
{
    return _width;
}

int Map::height() const
{
    return _height;
}

void Map::validate() const
{
    if(_width > 256)
    {
        throw runtime_error("map too wide");
    }

    if(_height > 256)
    {
        throw runtime_error("map too tall");
    }

    for(auto c : _data)
    {
        switch(c)
        {
            case '#': // wall
            case ' ': // space
            case '.': // pill
            case 'o': // power pill
            case '\\': // lambda man
            case '=': // ghost
            case '%': // fruit
                break;
            default:
                throw runtime_error("map has invalid character");
        }
    }

    bool missingEdge = false;

    for(auto x = 0; x < _width; x++)
    {
        missingEdge = missingEdge || (get({x, 0}) != '#');
        missingEdge = missingEdge || (get({x, _height - 1}) != '#');
    }

    for(auto y = 0; y < _height; y++)
    {
        missingEdge = missingEdge || (get({0, y}) != '#');
        missingEdge = missingEdge || (get({_width - 1, y}) != '#');
    }

    if(missingEdge)
    {
        throw runtime_error("map missing wall around edges");
    }

    for(auto x = 1; x < _width - 2; x++)
    {
        for(auto y = 1; y < _height - 2; y++)
        {
            if(get({x, y}) == ' ' && get({x + 1, y}) == ' ' &&
               get({x, y + 1}) == ' ' && get({x + 1, y + 1}) == ' ')
            {
                throw runtime_error("map has open area");
            }
        }
    }

    int numLambdaMen = 0;
    int numGhosts = 0;
    int numFruit = 0;

    for(auto x = 0; x < _width; x++)
    {
        for(auto y = 0; y < _height; y++)
        {
            switch(get({x, y}))
            {
                case '\\':
                    numLambdaMen++;
                    break;
                case '=':
                    numGhosts++;
                    break;
                case '%':
                    numFruit++;
                    break;
            }
        }
    }

    if(numLambdaMen != 1)
    {
        throw runtime_error("map has invalid number of lambda men");
    }

    if(numGhosts > 256)
    {
        throw runtime_error("map has invalid number of ghosts");
    }

    if(numFruit != 1)
    {
        throw runtime_error("map has an invalid number of fruit");
    }
}
