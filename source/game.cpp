#include "game.hpp"
#include <fstream>
#include <string>

struct EventComparer
{
    bool operator()(const Event& a, const Event& b)
    {
        if(a.clock.value > b.clock.value)
        {
            return true;
        }

        if(a.clock.value < b.clock.value)
        {
            return false;
        }

        if(a.type > b.type)
        {
            return true;
        }

        if(a.type < b.type)
        {
            return false;
        }

        return a.arg > b.arg;
    }
};

static bool operator!=(Clock a, Clock b)
{
    return a.value != b.value;
}

static const int PILL_VALUE = 10;
static const int POWER_PILL_VALUE = 50;
static const int FIRST_GHOST_VALUE = 200;
static const int MAX_GHOST_VALUE = 1600;

static int getFruitValue(int level)
{
    if(level <= 1)
        return 100;

    if(level <= 2)
        return 300;

    if(level <= 4)
        return 500;

    if(level <= 6)
        return 700;

    if(level <= 8)
        return 1000;

    if(level <= 10)
        return 2000;

    if(level <= 12)
        return 3000;

    return 5000;
}

void Game::init(const string& mapPath,
    const string& playerPath,
    const vector<string>& ghostPaths)
{
    {
        ifstream stream(mapPath);
        _map.init(stream);
    }

    _originalMap = _map;

    _ghosts.clear();
    _events.clear();
    _lives = 3;
    _score = 0;

    for(auto y = 0; y < _map.height(); y++)
    {
        for(auto x = 0; x < _map.width(); x++)
        {
            auto pos = Position {x, y};
            auto ch = _map.get(pos);

            if(ch == '\\')
            {
                ifstream stream(playerPath);
                _player.init(pos, stream);

                queuePlayerMove({0});
                _map.set(pos, ' ');
            }
            else if(ch == '=')
            {
                auto ghostNum = _ghosts.size();

                ifstream stream(ghostPaths[ghostNum % ghostPaths.size()]);
                _ghosts.emplace_back();
                _ghosts.back().init(ghostNum, pos, stream);

                queueGhostMove({0}, ghostNum);
                _map.set(pos, ' ');
            }
            else if(ch == '%')
            {
                _fruitPos = pos;
                _map.set(pos, ' ');
            }
        }
    }

    queueEvent({EventType::END_OF_LIVES, {127 * _map.width() * _map.height() * 16}, 0});
    queueEvent({EventType::FRUIT_APPEARS, {127 * 200}, 0});
    queueEvent({EventType::FRUIT_APPEARS, {127 * 400}, 0});
    queueEvent({EventType::FRUIT_EXPIRES, {127 * 280}, 0});
    queueEvent({EventType::FRUIT_EXPIRES, {127 * 480}, 0});
}

void Game::run()
{
    auto lastClock = Clock {};

    while(_lives != 0)
    {
        auto event = _events.front();
        pop_heap(_events.begin(), _events.end(), EventComparer{});
        _events.pop_back();

        if(event.clock != lastClock)
        {
            consume(lastClock);
            collide();
            dump(cout);
            getchar();

            if(_lives == 0)
            {
                // game over
                return;
            }
                
            if(remainingPills() == 0)
            {
                // game over, won
                _score *= _lives + 1;
                return;
            }
        }

        switch(event.type)
        {
            case EventType::END_OF_LIVES:
                _lives = 0;
                break;
            case EventType::FRUIT_APPEARS:
                _map.set(_fruitPos, '%');
                break;
            case EventType::FRUIT_EXPIRES:
                _map.set(_fruitPos, ' ');
                break;
            case EventType::FRIGHT_MODE_EXPIRES:
                for(auto& ghost : _ghosts)
                {
                    ghost.setInvisible(false);
                }
                break;
            case EventType::PLAYER_MOVES:
                _player.step(_map);
                queuePlayerMove(event.clock);
                break;
            case EventType::GHOST_MOVES:
                _ghosts[event.arg].step(*this);
                queueGhostMove(event.clock, event.arg);
                break;
        }
    }
}

const Map& Game::originalMap() const
{
    return _originalMap;
}

const Map& Game::map() const
{
    return _map;
}

const Player& Game::player() const
{
    return _player;
}

const Ghost& Game::ghost(int ghostNum) const
{
    return _ghosts[ghostNum];
}

void Game::consume(Clock thisClock)
{
    auto ch = _map.get(_player.position());

    if(ch == '.')
    {
        _map.set(_player.position(), ' ');
        _score += PILL_VALUE;
    }
    else if(ch == 'o')
    {
        _map.set(_player.position(), ' ');
        _score += POWER_PILL_VALUE;
        _ghostValue = FIRST_GHOST_VALUE;
        queueEvent({EventType::FRIGHT_MODE_EXPIRES, thisClock, 0});
    }
    else if(ch == '%')
    {
        _map.set(_player.position(), ' ');
        _score += getFruitValue(level());
    }
}

void Game::collide()
{
    for(auto& ghost : _ghosts)
    {
        if(ghost.invisible())
        {
            continue;
        }

        if(ghost.position() != _player.position())
        {
            continue;
        }

        if(frightMode())
        {
            ghost.setInvisible(true);
            ghost.reset();
            _score += _ghostValue;
            _ghostValue = min(_ghostValue * 2, MAX_GHOST_VALUE);
        }
        else
        {
            // TODO clear fright mode
            _player.reset();

            for(auto& ghost2 : _ghosts)
            {
                ghost2.setInvisible(false);
                ghost2.reset();
            }

            _lives--;
        }
    }
}

void Game::queuePlayerMove(Clock thisClock)
{
    auto moveTicks = 127 + (eating() ? 10 : 0);

    auto nextClock = Clock { thisClock.value + moveTicks };

    queueEvent({EventType::PLAYER_MOVES, nextClock, 0});
}

void Game::queueGhostMove(Clock thisClock, int ghostNum)
{
    auto moveTicks = 0;

    if(frightMode())
    {
        moveTicks = 195 + (ghostNum % 4) * 3;
    }
    else
    {
        moveTicks = 130 + (ghostNum % 4) * 2;
    }

    auto nextClock = Clock { thisClock.value + moveTicks };

    queueEvent({EventType::GHOST_MOVES, nextClock, ghostNum});
}

void Game::queueEvent(Event event)
{
    _events.push_back(event);
    push_heap(_events.begin(), _events.end(), EventComparer{});
}

bool Game::eating() const
{
    return _map.get(_player.position()) != ' ';
}

bool Game::frightMode() const
{
    for(auto& event : _events)
    {
        if(event.type == EventType::FRIGHT_MODE_EXPIRES)
        {
            return true;
        }
    }

    return false;
}

int Game::level() const
{
    return _map.width() * _map.height() / 100 + 1;
}

int Game::remainingPills() const
{
    auto numPills = 0;

    for(auto y = 0; y < _map.height(); y++)
    {
        for(auto x = 0; x < _map.width(); x++)
        {
            auto ch = _map.get({x, y});

            if(ch == '.' || ch == 'o')
            {
                numPills++;
            }
        }
    }

    return numPills;
}

void Game::dump(ostream& os) const
{
    for(auto y = 0; y < _map.height(); y++)
    {
        for(auto x = 0; x < _map.width(); x++)
        {
            auto pos = Position {x, y};
            char ch = _map.get(pos);

            if(_player.position() == pos)
            {
                ch = '\\';
            }

            for(auto& ghost : _ghosts)
            {
                if(ghost.position() == pos)
                {
                    ch = '=';
                }
            }

            os << ch;
        }

        os << '\n';
    }
}
