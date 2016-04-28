#pragma once
// Based on the Example availble at: http://ngildea.blogspot.co.uk/2014/11/implementing-dual-contouring.html
// Github Repo with example code: https://github.com/nickgildea/DualContouringSample
// Personal Fork: https://github.com/Markyparky56/DualContouringSample (minor changes)

#include <glm\glm.hpp>
using glm::ivec3;
using glm::vec3;

#include "QEF.hpp"
#include "Mesh.hpp"

#include <array>

// Material constants
const int MaterialAir = 0;
const int MaterialSolid = 1;

// Quadratic Error Function constants
const float QefError = 1e-6f;
const int QefSweeps = 4;


const ivec3 ChildMinOffsets[] =
{
    // Needs to match the vertMap from Dual Contouring implemntation
    ivec3(0, 0, 0),
    ivec3(0, 0, 1),
    ivec3(0, 1, 0),
    ivec3(0, 1, 1),
    ivec3(1, 0, 0),
    ivec3(1, 0, 1),
    ivec3(1, 1, 0),
    ivec3(1, 1, 1)
};

// ----------------------------------------------------------------------------
// data from the original DC impl, drives the contouring process

template<typename T, int X, int Y>        using Array2D = std::array<std::array<T, X>, Y>;
template<typename T, int X, int Y, int Z> using Array3D = std::array<std::array<std::array<T, X>, Y>, Z>;

const Array2D<int, 2, 12> edgevmap
{{
    { 0,4 },{ 1,5 },{ 2,6 },{ 3,7 },	// x-axis 
    { 0,2 },{ 1,3 },{ 4,6 },{ 5,7 },	// y-axis
    { 0,1 },{ 2,3 },{ 4,5 },{ 6,7 }		// z-axis
}};

const std::array<int, 3> edgemask { 5, 3, 6 };

const Array2D<int, 3, 8> vertMap
{{
    { 0,0,0 },
    { 0,0,1 },
    { 0,1,0 },
    { 0,1,1 },
    { 1,0,0 },
    { 1,0,1 },
    { 1,1,0 },
    { 1,1,1 }
}};

const Array2D<int, 4, 6> faceMap {{ { 4, 8, 5, 9 },{ 6, 10, 7, 11 },{ 0, 8, 1, 10 },{ 2, 9, 3, 11 },{ 0, 4, 2, 6 },{ 1, 5, 3, 7 } }};
const Array2D<int, 3, 12> cellProcFaceMask {{ { 0,4,0 },{ 1,5,0 },{ 2,6,0 },{ 3,7,0 },{ 0,2,1 },{ 4,6,1 },{ 1,3,1 },{ 5,7,1 },{ 0,1,2 },{ 2,3,2 },{ 4,5,2 },{ 6,7,2 } }};
const Array2D<int, 5, 6> cellProcEdgeMask {{ { 0,1,2,3,0 },{ 4,5,6,7,0 },{ 0,4,1,5,1 },{ 2,6,3,7,1 },{ 0,2,4,6,2 },{ 1,3,5,7,2 } }};

const Array3D<int, 3, 4, 3> faceProcFaceMask 
{{
    {{ { 4,0,0 },{ 5,1,0 },{ 6,2,0 },{ 7,3,0 } }},
    {{ { 2,0,1 },{ 6,4,1 },{ 3,1,1 },{ 7,5,1 } }},
    {{ { 1,0,2 },{ 3,2,2 },{ 5,4,2 },{ 7,6,2 } }}
}};

const Array3D<int, 6, 4, 3> faceProcEdgeMask
{{
    {{ { 1,4,0,5,1,1 },{ 1,6,2,7,3,1 },{ 0,4,6,0,2,2 },{ 0,5,7,1,3,2 } }},
    {{ { 0,2,3,0,1,0 },{ 0,6,7,4,5,0 },{ 1,2,0,6,4,2 },{ 1,3,1,7,5,2 } }},
    {{ { 1,1,0,3,2,0 },{ 1,5,4,7,6,0 },{ 0,1,5,0,4,1 },{ 0,3,7,2,6,1 } }}
}};

const Array3D<int, 5, 2, 3> edgeProcEdgeMask 
{{
    {{ { 3,2,1,0,0 },{ 7,6,5,4,0 } }},
    {{ { 5,1,4,0,1 },{ 7,3,6,2,1 } }},
    {{ { 6,4,2,0,2 },{ 7,5,3,1,2 } }},
}};

const Array2D<int, 4, 3> processEdgeMask{{ { 3,2,1,0 },{ 7,5,6,4 },{ 11,10,9,8 } }};

// -------------------------------------------------------------------------------


// OctreeNode Class
enum class OctreeNodeType
{
    None,
    Internal,
    Pseudo,
    Leaf
};

struct OctreeDrawInfo
{
    OctreeDrawInfo()
        : index(-1)
        , corners(0)
    {
    }

    int index;
    int corners;
    vec3 position;
    vec3 averageNormal;
    svd::QefData qef;
};

class OctreeNode
{
public:
    OctreeNode()
        : OctreeNode(OctreeNodeType::None)
    {
    }

    OctreeNode(const OctreeNodeType InType)
        : type(InType)
        , min(0, 0, 0)
        , size(0)
        , drawInfo(nullptr)
    {
        for (int i = 0; i < 8; i++)
        {
            children[i] = nullptr;
        }
    }

public:
    OctreeNodeType  type;
    ivec3           min;
    int             size;
    OctreeNode*     children[8];
    OctreeDrawInfo* drawInfo;
};

OctreeNode* BuildOctree(const ivec3& min, const int size, const float threshold);
void DestroyOctree(OctreeNode* node);
void GenerateMeshFromOctree(OctreeNode* node, VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer);
