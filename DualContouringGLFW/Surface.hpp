#pragma once
#include "Octree.hpp"
#include "Mesh.hpp"

struct Surface
{
    OctreeNode *root;
    Mesh *mesh;
};
