#pragma once
#include "ThreadTask.hpp"

// Forward Declare the ThreadFarm so we can be BFFs
class ThreadFarm;

// Worker thread pulls tasks from the task queue in ThreadFarm
class WorkerThread
{
public:
    static void WorkerFunction()
    {
        while (true)
        {

        }
    }

private:
    friend ThreadFarm;
};
