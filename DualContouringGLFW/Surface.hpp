#pragma once
#include "Octree.hpp"
#include "Mesh.hpp"
#include <memory>

struct Surface
{
    Surface()
        : root(nullptr)
        , mesh(nullptr)
        , position(0.f, 0.f, 0.f)
    {}
    virtual ~Surface() 
    { 
        DestroyOctree(root); 
        mesh->Destroy();
    }
    OctreeNode *root;
    std::unique_ptr<Mesh> mesh;
    vec3 position;
};
