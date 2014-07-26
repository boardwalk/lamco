#ifndef LAMCO_BASIC_HPP
#define LAMCO_BASIC_HPP

enum class Direction
{
    UP,
    RIGHT,
    DOWN,
    LEFT
};

struct Position
{
    int x;
    int y;

    Position move(Direction direction) const
    {
        switch(direction)
        {
            case Direction::UP:
                return {x, y - 1};
            case Direction::RIGHT:
                return {x + 1, y};
            case Direction::DOWN:
                return {x, y + 1};
            case Direction::LEFT:
                return {x - 1, y};
        }

        return {x, y};
    }
};

inline bool operator==(Position a, Position b)
{
    return a.x == b.x && a.y == b.y;
}

inline bool operator!=(Position a, Position b)
{
    return a.x != b.x || a.y != b.y;
}

#endif
