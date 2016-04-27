#pragma once
#include <thread>
#include <vector>
#include <memory>
#include <queue>
#include <mutex>

#include "ThreadTask.hpp"
#include "WorkerThread.hpp"

// Simple Thread Farm
// Creates a specified number of worker threads
// Queues tasks which are allocated to worker threads

class ThreadFarm
{
public:
    ThreadFarm();
    virtual ~ThreadFarm();

    void Run(unsigned int NumWorkerThreads);
    void PushNewTask(ThreadTask *InTask);
    void WorkerThread();

protected:
    using pThread = std::unique_ptr<std::thread>;
    std::vector<pThread> workers;
    std::queue<ThreadTask*> taskQueue;
    std::mutex taskQueueMutex;
};