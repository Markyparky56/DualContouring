#include "app.hpp"
#include <string>

int main(int argc, char *argv[])
{
    bool testMode = false;
    int numThreads = -1;
    std::vector<std::string> switches;
    if (argc > 1)
    {
        for (int i = 1; i < argc; i++)
        {
            switches.push_back(argv[i]);
        }
    }
    int numSwitches = static_cast<int>(switches.size());
    for (int i = 0; i < numSwitches;)
    {
        if (switches[i] == "/test")
        {
            testMode = true;
            std::cout << "Enabling Test Mode\n";
            i++;
        }
        else if (switches[i] == "/threads")
        {
            if (i != numSwitches)
            {
                auto numStr = switches[i + 1];
                numThreads = std::stol(numStr);
                std::cout << "Setting number of threads to " << numThreads << std::endl;
                i += 2;
            }
        }
        else if (switches[i] == "/octreesize")
        {
            if (i != numSwitches) // Ensure we don't go out of bounds
            {
                auto sizeStr = switches[i + 1];
                OctreeSize = std::stol(sizeStr);
                std::cout << "Setting OctreeSize to " << OctreeSize << std::endl;
                i += 2;
            }
        }
        else
        {
            i++; // Ignore the unrecognised switch
        }
    }
    App app;
    int result = app.Init(numThreads);
    if (result == EXIT_FAILURE) printf("Exit Failure During App Initialisation!\n");
    else app.Run(testMode);
    return 0;
}
