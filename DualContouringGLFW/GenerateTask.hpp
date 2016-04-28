#pragma once
#include <iostream>
#include <iomanip>
#include "ThreadTask.hpp"
#include "Surface.hpp"

#include "ContourTask.hpp"
#include "ThreadFarm.hpp"

// GenerateTask will create a ContourTask after completing it's Octree

class GenerateTask : public ThreadTask
{
public:
    struct Args : public TaskArgs
    {
        Surface *surface = nullptr;
        ThreadFarm *farm = nullptr;
        glm::ivec3 min;
        int size;
        float threshold;
    protected:
        // No point exposing this to the app since it's set by the generate task
        ContourTask::Args *contourTaskPtr = nullptr;
        friend GenerateTask; // BFFs!
    };

    inline void Run() override
    {
        assert(args != nullptr);
        Args *taskArgs = static_cast<Args*>(args);
        assert(taskArgs->surface != nullptr);
        assert(taskArgs->farm != nullptr);

#if defined(_DEBUG) | defined(WITH_TIMINGS)
        auto start = clock.now();
#endif // _DEBUG
        
        // The Actual Task
        taskArgs->surface->root = BuildOctree(taskArgs->min, taskArgs->size, taskArgs->threshold);

#if defined(_DEBUG) | defined(WITH_TIMINGS)
        auto end = clock.now();
        taskArgs->timeTaken = end - start;
        outputMutex.lock();
        std::cout << "Task ID: " << id << " completed in " << std::setprecision(5) << taskArgs->timeTaken.count() << std::endl;;
        outputMutex.unlock();
#endif // _DEBUG

        // Queue a new task
        ThreadTask *contourTask = new ContourTask;
        ContourTask::Args *contourTaskArgs = new ContourTask::Args;
        contourTaskArgs->surface = taskArgs->surface;
        taskArgs->contourTaskPtr = contourTaskArgs;
        contourTask->SetArgs(contourTaskArgs);
        taskArgs->farm->PushNewTask(contourTask);
    }

private:

};
