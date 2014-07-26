#ifndef LAMCO_GHOST_HPP
#define LAMCO_GHOST_HPP

#include "basic.hpp"
#include "map.hpp"
#include <iostream>

using namespace std;

enum class GhcOpcode : uint8_t
{
    MOV, INC, DEC, ADD, SUB, MUL, DIV, AND,
    OR,  XOR, JLT, JEQ, JGT, INT, HLT
};

struct GhcArgument
{
    bool isIndirect;
    bool isRegister;
    uint8_t value;
};

struct GhcInstruction
{
    GhcOpcode opcode;
    GhcArgument arg1;
    GhcArgument arg2;
    GhcArgument arg3;
};

class Game;

class Ghost
{
public:
    void init(int ghostNum, Position pos, istream& is);

    void step(const Game& game);
    void kill();
    void reset();

    Position position() const;
    bool dead() const;

private:
    void run(const Game& game);
    void handleInterrupt(const Game& game, int num);
    uint8_t load(GhcArgument arg) const;
    void store(GhcArgument arg, uint8_t value);

    int _ghostNum;
    Position _startPosition;
    Position _position;
    Direction _direction;
    bool _dead;

    vector<uint8_t> _registers;
    vector<uint8_t> _data;    
    vector<GhcInstruction> _code;
};

#endif