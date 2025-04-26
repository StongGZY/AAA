#include "game_client/game_client.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
        return 1;
    }

    GameClient client(argv[1], std::atoi(argv[2]));
    if (client.initialize() != GameClient::SUCCESS)
    {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return 1;
    }

    client.run();
    return 0;
}