#ifndef LAMCO_GAME_HPP
#define LAMCO_GAME_HPP

#include "Map.hpp"
#include "Player.hpp"
#include "Ghost.hpp"

using namespace std;

struct Clock
{
    int value;
};

enum class EventType
{
    // Events here occur in the order listed here when on the same tick
    END_OF_LIVES,
    PLAYER_MOVES,
    GHOST_MOVES,
    FRUIT_APPEARS,
    FRUIT_EXPIRES,
    FRIGHT_MODE_EXPIRES
};

struct Event
{
    EventType type;
    Clock clock;
    int arg;
};

class Game
{
public:
    void init(const string& mapPath,
        const string& playerPath,
        const vector<string>& ghostPaths);
    void run();

    const Map& originalMap() const;
    const Map& map() const;
    const Player& player() const;
    const Ghost& ghost(int ghostNum) const;
    bool frightMode() const;

private:
    void consume(Clock thisClock);
    void collide();
    void queuePlayerMove(Clock thisClock);
    void queueGhostMove(Clock thisClock, int ghostNum);
    void queueEvent(Event event);

    bool eating() const;
    int level() const;
    int remainingPills() const;

    void dump(ostream& os) const;

    Map _originalMap;
    Map _map;
    Player _player;
    vector<Ghost> _ghosts;
    vector<Event> _events;
    Position _fruitPos;
    int _lives;
    int _score;
    int _ghostValue;
};

#endif