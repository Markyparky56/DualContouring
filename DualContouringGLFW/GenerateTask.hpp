#pragma once
#include "ThreadTask.hpp"
#include "Surface.hpp"

#include "ContourTask.hpp"
#include "ThreadFarm.hpp"

// GenerateTask will create a ContourTask after completing it's Octree

class GenerateTask : public ThreadTask
{
public:
    struct GenerateTaskArgs : public TaskArgs
    {
        Surface *surface = nullptr;
        ThreadFarm *farm = nullptr;
        glm::ivec3 min;
        int size;
        float threshold;
        ContourTask::ContourTaskArgs *contourTaskPtr = nullptr;
    };

    inline void Run() override
    {
        assert(args != nullptr);
        GenerateTaskArgs *taskArgs = static_cast<GenerateTaskArgs*>(args);
        assert(taskArgs->surface != nullptr);
        assert(taskArgs->farm != nullptr);

#ifdef _DEBUG
        auto start = clock.now();
#endif // _DEBUG

        // The Actual Task
        taskArgs->surface->root = BuildOctree(taskArgs->min, taskArgs->size, taskArgs->threshold);

#ifdef _DEBUG
        auto end = clock.now();
        taskArgs->timeTaken = end - start;
        printf("Task ID: %d completed in %.1fs\n", id, taskArgs->timeTaken);
#endif // _DEBUG

        // Queue a new task
        ThreadTask *contourTask = new ContourTask;
        ContourTask::ContourTaskArgs *contourTaskArgs = new ContourTask::ContourTaskArgs;
        contourTaskArgs->surface = taskArgs->surface;
        taskArgs->contourTaskPtr = contourTaskArgs;
        contourTask->SetArgs(contourTaskArgs);
        taskArgs->farm->PushNewTask(contourTask);
    }

private:

};
