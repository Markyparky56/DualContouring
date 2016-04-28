#include "app.hpp"
#include <string>

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        OctreeSize = std::stol(argv[1]);
        if (OctreeSize < 0) OctreeSize = 64;
    }
    App app;
    int result = app.Init();
    if (result == EXIT_FAILURE) printf("Exit Failure During App Initialisation!\n");
    else app.Run();
    return 0;
}
