#include "game.hpp"
#include <getopt.h>

static const option long_options[] =
{
    {"map", required_argument, nullptr, 'm'},
    {"player", required_argument, nullptr, 'p'},
    {"ghost", required_argument, nullptr, 'g'},
    {nullptr, 0, nullptr, '\0'}
};

int main(int argc, char* argv[])
{
    try
    {
        string mapPath;
        string playerPath;
        vector<string> ghostPaths;

        while(true)
        {
            int index;
            auto opt = getopt_long(argc, argv, "m:p:g:", long_options, &index);

            if(opt < 0)
            {
                break;
            }

            switch(opt)
            {
                case 'm':
                    mapPath = optarg;
                    break;
                case 'p':
                    playerPath = optarg;
                    break;
                case 'g':
                    ghostPaths.push_back(optarg);
                    break;
            }
        }

        if(mapPath.empty())
        {
            throw runtime_error("--map, -m argument required");
        }

        if(playerPath.empty())
        {
            throw runtime_error("--player, -p argument required");
        }

        if(ghostPaths.empty())
        {
            throw runtime_error("--ghost, -g argument required");
        }

        Game game;
        game.init(mapPath, playerPath, ghostPaths);
        game.run();
    }
    catch(const runtime_error& e)
    {
        cerr << "An error occurred: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}