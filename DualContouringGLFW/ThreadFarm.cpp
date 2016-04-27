#include "ThreadFarm.hpp"
#include <functional>

ThreadFarm::ThreadFarm()
{
}

ThreadFarm::~ThreadFarm()
{
}

void ThreadFarm::Run(unsigned int NumWorkerThreads)
{
    for (int i = 0; i < NumWorkerThreads; i++)
    {
        workers.push_back(pThread(new std::thread(std::mem_fun(&ThreadFarm::WorkerThread), this)));
    }
    for (auto& worker : workers)
    {
        worker->join();
    }
}

void ThreadFarm::PushNewTask(ThreadTask *InTask)
{
    taskQueue.push(InTask);
}

void ThreadFarm::WorkerThread()
{
    ThreadTask *task;
    while (true)
    {
        // Grab a task
        {
            taskQueueMutex.lock();
            if (!taskQueue.empty())
            {
                task = taskQueue.front();
                taskQueue.pop();
                taskQueueMutex.unlock();
            }
            else
            {
                taskQueueMutex.unlock();
                break;
            }
        }
        // Execute it
        task->Run();
        // Delete it
        delete task;
    }
}
