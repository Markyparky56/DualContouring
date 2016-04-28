#pragma once
#include <thread>
#include <vector>
#include <memory>
#include <queue>
#include <mutex>
#include <atomic>
#include "IdPool.hpp"
#include "ThreadTask.hpp"
using pThread = std::unique_ptr<std::thread>;



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

    //bool Working() 
    //{ 
    //    bool emptyQueue = taskQueue.empty();
    //    bool working = false;
    //    activeWorkersMutex.lock();
    //    for (auto& activeWorker : activeWorkers)
    //    {
    //        working = activeWorker;
    //        if (working) break;
    //    }
    //    activeWorkersMutex.unlock();
    //    return emptyQueue && !working;
    //}

protected:
    void WorkerThread();
    IdPool workerIdPool;
    std::vector<pThread> workers;
    // vector of atomics not possible
    //std::vector<std::atomic<bool>> activeWorkers; 
    std::mutex activeWorkersMutex;
    std::queue<ThreadTask*> taskQueue;
    std::mutex taskQueueMutex;
};