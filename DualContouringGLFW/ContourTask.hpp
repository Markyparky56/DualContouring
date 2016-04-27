#pragma once
#include "ThreadTask.hpp"
#include "Octree.hpp"
#include "Mesh.hpp"

class ContourTask : public ThreadTask
{
public:
    struct ContourTaskArgs : public TaskArgs
    {
        Surface *surface;
    };

    inline void Run() override
    {
        assert(args != nullptr);
        ContourTaskArgs *taskArgs = static_cast<ContourTaskArgs*>(args);
        assert(taskArgs->surface != nullptr);

#ifdef _DEBUG
        auto start = clock.now();
#endif // _DEBUG

        // The Actual Task
        GenerateMeshFromOctree(taskArgs->surface->root, vertices, indices);
        taskArgs->surface->mesh->UploadData(vertices, indices);

#ifdef _DEBUG
        auto end = clock.now();
        taskArgs->timeTaken = end - start;
        printf("Task ID: %d completed in %.1fs\n", id, taskArgs->timeTaken);
#endif // _DEBUG
    }

private:
    VertexBuffer vertices;
    IndexBuffer indices;
};
