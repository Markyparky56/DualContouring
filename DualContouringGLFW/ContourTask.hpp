#pragma once
#include "ThreadTask.hpp"
#include "Octree.hpp"
#include "Mesh.hpp"
#include <sstream>

class ContourTask : public ThreadTask
{
public:
    struct Args : public TaskArgs
    {
        Surface *surface;
    };

    inline void Run() override
    {
        assert(args != nullptr);
        Args *taskArgs = static_cast<Args*>(args);
        assert(taskArgs->surface != nullptr);

#if defined(_DEBUG) | defined(WITH_TIMINGS)
        auto start = clock.now();
#endif // _DEBUG

        // The Actual Task
        GenerateMeshFromOctree(taskArgs->surface->root, vertices, indices);
        taskArgs->surface->mesh = std::unique_ptr<Mesh>(new Mesh);
        taskArgs->surface->mesh->UploadData(vertices, indices);

#if defined(_DEBUG) | defined(WITH_TIMINGS)
        auto end = clock.now();
        taskArgs->timeTaken = end - start;
        {            
            std::stringstream ss;
            ss << "Task ID: " << id << " completed in " << std::setprecision(5) << taskArgs->timeTaken.count() << std::endl;;
            std::unique_lock<std::mutex> lock(outputMutex);
            args->outputChannel->Write(ss.str());
        }
#endif // _DEBUG
    }

private:
    VertexBuffer vertices;
    IndexBuffer indices;
};
