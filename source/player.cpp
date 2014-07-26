#include "player.hpp"

void Player::init(Position pos, istream& is)
{
    _startPosition = pos;
    _position = pos;
    _direction = Direction::DOWN;

    // TODO
    (void)is;
}

void Player::step(const Map& map)
{
    // TODO run CPU here

    auto newPos = _position.move(_direction);

    if(map.get(newPos) != '#')
    {
        _position = newPos;
    }
}

void Player::reset()
{
    _position = _startPosition;
}

Position Player::position() const
{
    return _position;
}