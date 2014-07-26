#include "ghost.hpp"
#include "game.hpp"
#include <sstream>

static const int MAX_INSTR_COUNT = 1024;

static GhcOpcode parseOpcode(const string& str)
{
    if(str == "mov") { return GhcOpcode::MOV; }
    else if(str == "inc") { return GhcOpcode::INC; }
    else if(str == "dec") { return GhcOpcode::DEC; }
    else if(str == "add") { return GhcOpcode::ADD; }
    else if(str == "sub") { return GhcOpcode::SUB; }
    else if(str == "mul") { return GhcOpcode::MUL; }
    else if(str == "div") { return GhcOpcode::DIV; }
    else if(str == "and") { return GhcOpcode::AND; }
    else if(str == "or") { return GhcOpcode::OR; }
    else if(str == "xor") { return GhcOpcode::XOR; }
    else if(str == "jlt") { return GhcOpcode::JLT; }
    else if(str == "jeq") { return GhcOpcode::JEQ; }
    else if(str == "jgt") { return GhcOpcode::JGT; }
    else if(str == "int") { return GhcOpcode::INT; }
    else if(str == "hlt") { return GhcOpcode::HLT; }

    throw runtime_error("unknown opcode");
}

static GhcArgument parseArgument(const string& str)
{
    auto arg = GhcArgument {false, false, 0};

    auto pos = 0u;

    if(pos + 1 > str.size())
    {
        return arg;
    }

    if(str[pos] == '[')
    {
        arg.isIndirect = true;
        pos++;
    }

    if(pos + 1 > str.size())
    {
        throw runtime_error("argument too short");
    }

    if(str[pos] >= 'a' && str[pos] <= 'h')
    {
        arg.isRegister = true;
        arg.value = str[pos] - 'a' + 1;
    }
    else if(str[pos] == 'p')
    {
        arg.isRegister = true;
        arg.value = 0;
    }
    else
    {
        arg.value = atoi(str.c_str() + pos);
    }

    return arg;
}

void Ghost::init(int ghostNum, Position pos, istream& is)
{
    _ghostNum = ghostNum;
    _startPosition = pos;
    _position = pos;
    _direction = Direction::DOWN;
    _dead = false;

    if(!is)
    {
        throw runtime_error("bad input stream");
    }

    _registers.clear();
    _registers.resize(9); // PC, A-H
    _data.clear();
    _data.resize(0xFF);
    _code.clear();

    while(is)
    {
        string str;
        getline(is, str);

        // remove comments
        auto commentPos = str.find(';');

        if(commentPos != string::npos)
        {
            str = str.substr(0, commentPos);
        }

        // lowercase and remove commas
        for(auto& c : str)
        {
            c = tolower(c);

            if(c == ',')
            {
                c = ' ';
            }
        }

        auto mnemonic = string {};
        auto arg1 = string {};
        auto arg2 = string {};
        auto arg3 = string {};
        stringstream(str) >> mnemonic >> arg1 >> arg2 >> arg3;

        if(mnemonic.empty())
        {
            continue;
        }

        _code.push_back({
            parseOpcode(mnemonic),
            parseArgument(arg1),
            parseArgument(arg2),
            parseArgument(arg3)
        });
    }
}

void Ghost::step(const Game& game)
{
    run(game);

    Direction directions[] =
    {
        _direction,
        Direction::UP,
        Direction::RIGHT,
        Direction::DOWN,
        Direction::LEFT
    };

    for(auto i = 0u; i < sizeof(directions)/sizeof(directions[0]); i++)
    {
        auto newPos = _position.move(directions[i]);

        if(game.map().get(newPos) != '#')
        {
            _direction = directions[i];
            _position = newPos;
            break;
        }
    }
}

void Ghost::reset()
{
    _position = _startPosition;
}

void Ghost::kill()
{
    _dead = true;
}

Position Ghost::position() const
{
    return _position;
}

bool Ghost::dead() const
{
    return _dead;
}

void Ghost::run(const Game& game)
{
    auto stepCount = 0;

    while(stepCount < MAX_INSTR_COUNT)
    {
        auto nextPc = _registers[0];
        auto instr = _code[nextPc++];

        switch(instr.opcode)
        {
            case GhcOpcode::MOV:
                store(instr.arg1, load(instr.arg2));
                break;
            case GhcOpcode::INC:
                store(instr.arg1, load(instr.arg1) + 1);
                break;
            case GhcOpcode::DEC:
                store(instr.arg1, load(instr.arg1) - 1);
                break;
            case GhcOpcode::ADD:
                store(instr.arg1, load(instr.arg1) + load(instr.arg2));
                break;
            case GhcOpcode::SUB:
                store(instr.arg1, load(instr.arg1) - load(instr.arg2));
                break;
            case GhcOpcode::MUL:
                store(instr.arg1, load(instr.arg1) * load(instr.arg2));
                break;
            case GhcOpcode::DIV:
                store(instr.arg1, load(instr.arg1) / load(instr.arg2));
                break;
            case GhcOpcode::AND:
                store(instr.arg1, load(instr.arg1) & load(instr.arg2));
                break;
            case GhcOpcode::OR:
                store(instr.arg1, load(instr.arg1) | load(instr.arg2));
                break;
            case GhcOpcode::XOR:
                store(instr.arg1, load(instr.arg1) ^ load(instr.arg2));
                break;
            case GhcOpcode::JLT:
                if(load(instr.arg2) < load(instr.arg3))
                {
                    nextPc = load(instr.arg1);
                }
                break;
            case GhcOpcode::JEQ:
                if(load(instr.arg2) == load(instr.arg3))
                {
                    nextPc = load(instr.arg1);
                }
                break;
            case GhcOpcode::JGT:
                if(load(instr.arg2) > load(instr.arg3))
                {
                    nextPc = load(instr.arg1);
                }
                break;
            case GhcOpcode::INT:
                handleInterrupt(game, load(instr.arg1));
                break;
            case GhcOpcode::HLT:
                return;
        }

        _registers[0] = nextPc;
        stepCount++;
    }
}

void Ghost::handleInterrupt(const Game& game, int num)
{
    switch(num)
    {
        case 0:
            // set direction
            if(_registers[1] < 4)
            {
                _direction = (Direction)_registers[1];
            }
            break;
        case 1:
            // get player position
            _registers[1] = game.player().position().x;
            _registers[2] = game.player().position().y;
            break;
        case 2:
            // get player 2 position
            break;
        case 3:
            // get ghost index
            _registers[1] = _ghostNum;
            break;
        case 4:
            // get ghost start pos
            _registers[1] = _startPosition.x;
            _registers[2] = _startPosition.y;
            break;
        case 5:
            // get ghost current pos
            _registers[1] = _position.x;
            _registers[2] = _position.y;
            break;
        case 6:
            // get ghost direction and status
            _registers[1] = (uint8_t)_direction;
            _registers[2] = _dead; // FIXME
            break;
        case 7:
            // get map square
            {
                auto pos = Position { _registers[1], _registers[2] };
                auto ch = '#';
                auto value = uint8_t {};

                if(pos.x < game.originalMap().width() && pos.y < game.originalMap().height())
                {
                    ch = game.originalMap().get(pos);
                }

                switch(ch)
                {
                    case '#':
                        value = 0;
                        break;
                    case ' ':
                        value = 1;
                        break;
                    case '.':
                        value = 2;
                        break;
                    case 'o':
                        value = 3;
                        break;
                    case '%':
                        value = 4;
                        break;
                    case '\\':
                        value = 5;
                        break;
                    case '=':
                        value = 6;
                        break;
                }

                _registers[1] = value;
            }
            break;
        case 8:
            // trace
            cerr << "register dump:";
            for(auto r : _registers)
            {
                cerr << " " << r;
            }
            cerr << endl;
            break;
        default:
            throw runtime_error("unknown interrupt");
    }
}

uint8_t Ghost::load(GhcArgument arg) const
{
    auto value = arg.value;

    if(arg.isRegister)
    {
        value = _registers[value];
    }

    if(arg.isIndirect)
    {
        value = _data[value];
    }

    return value;
}

void Ghost::store(GhcArgument arg, uint8_t value)
{
    if(arg.isIndirect)
    {
        if(arg.isRegister)
        {
            _data[_registers[arg.value]] = value;
        }
        else
        {
            _data[arg.value] = value;
        }
    }
    else
    {
        if(arg.isRegister)
        {
            _registers[arg.value] = value;
        }
        else
        {
            throw logic_error("cannot store to constant");
        }
    }
}