#pragma once
#include <memory>
#if defined(_DEBUG) | defined(WITH_TIMINGS)
#include <chrono>
#include <mutex>
    using hiResClock = std::chrono::high_resolution_clock;
    using std::chrono::milliseconds;
    using std::chrono::duration_cast;
    using fsec = std::chrono::duration<float>;
    static std::mutex outputMutex;
#endif // _DEBUG

// Base Thread Task
// Friends with the ThreadFarm
class ThreadTask
{
public:
    ThreadTask() 
        : args(nullptr)
    {
        id = TaskCount++; 
    }
    virtual ~ThreadTask() {};

    struct TaskArgs 
    {
#if defined(_DEBUG) | defined(WITH_TIMINGS)
        fsec timeTaken;
#endif // _DEBUG
    };
    inline void SetArgs(TaskArgs *InArgs) { args = InArgs; }

    virtual void Run() = 0;
protected:
    static unsigned int TaskCount;
    unsigned int id;
    TaskArgs *args;
#if defined(_DEBUG) | defined(WITH_TIMINGS)
    hiResClock clock;
#endif // _DEBUG
};
