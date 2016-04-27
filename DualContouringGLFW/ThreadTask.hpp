#pragma once
#include <memory>
#ifdef _DEBUG
    #include <chrono>
    using hiResClock = std::chrono::high_resolution_clock;
    using std::chrono::milliseconds;
    using std::chrono::duration_cast;
    using fsec = std::chrono::duration<float>;
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
#ifdef _DEBUG
        fsec timeTaken;
#endif // _DEBUG
    };
    inline void SetArgs(TaskArgs *InArgs) { args = InArgs; }

    virtual void Run() = 0;
protected:
    static unsigned int TaskCount;
    unsigned int id;
    TaskArgs *args;
#ifdef _DEBUG
    hiResClock clock;
#endif // _DEBUG
};
