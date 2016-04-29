#pragma once
#include <thread>
#include <GL\glew.h>
#include <glfw3.h>
#include <glm\glm.hpp>
#include <glm\ext.hpp>
#include <memory>
#include <chrono>

#include "ThreadFarm.hpp"
#include "Surface.hpp"
#include "Channel.hpp"

#define DegToRad(deg)((glm::pi<float>() * deg)/180.0f)

using hiResClock = std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using std::chrono::duration_cast;
using fsec = std::chrono::duration<float>;

const int WindowWidth = 1280;
const int WindowHeight = 720;
static int OctreeSize = 64; // Smaller == Faster TODO: O(8^n)?

class DrawQueueSort
{
public:
    static vec3 camPos;

    bool operator()(Surface *lhs, Surface *rhs)
    {
        vec3 diffLhs = lhs->position - camPos;
        vec3 diffRhs = rhs->position - camPos;
        float distanceLhs = std::sqrtf(glm::dot(diffLhs, diffLhs));
        float distanceRhs = std::sqrtf(glm::dot(diffRhs, diffRhs));

        return distanceLhs < distanceRhs;
    }
};

// App class to encapsulate the functionality of the program
class App
{
public:
    App();
    ~App();

    int Init(const int InNumThreads);
    int Run(const bool TestMode);

private:
    hiResClock clock;
    hiResClock::time_point prev;
    fsec delta;
    std::mutex deltaMutex;

    bool running;
    bool regenerateSurfaces, regeneratingSurfaces;
    bool wireframeMode;
    std::mutex wireframeModeMutex;

    double runTime;
    float optionToggleGracePeriod;
    // Time since last option change
    float wireframeModeTime, regenerateTime;

    void RenderThread();
    void OutputThread();
    void DrawSurface(Surface* surface);
    void GenerateSurfaces(float Threshold);

    std::array< std::array< std::unique_ptr<Surface>, 3>, 3> surfaces;
    std::priority_queue<Surface*, std::deque<Surface*>, DrawQueueSort> drawQueue;

    GLFWwindow *window;
    GLuint shaderProgramID;

    pThread renderThread;
    pThread outputThread;

    int numThreads;
    std::unique_ptr<ThreadFarm> farm;
    std::unique_ptr< Channel<std::string> > outputChannel;
};
