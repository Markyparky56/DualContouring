#pragma once
#include <glm\glm.hpp>
#include <GL\glew.h>


#include <vector>

struct MeshVertex
{
    MeshVertex(const glm::vec3& InXYZ, const glm::vec3& InNormal)
        : xyz(InXYZ)
        , normal(InNormal)
    {}

    glm::vec3 xyz, normal;
};

using VertexBuffer = std::vector<MeshVertex>;
using IndexBuffer = std::vector<int>;

class Mesh
{
public:
    Mesh()
        : vertexArrayObj(-1)
        , vertexBuffer(-1)
        , indexBuffer(-1)
        , numIndices(0)
        , buffersGenerated(false)
    {}

    void Initialise();
    void UploadData(const VertexBuffer& vertices, const IndexBuffer& indices);
    void Bind();
    void Destroy();

    bool buffersGenerated;

    GLuint vertexArrayObj, vertexBuffer, indexBuffer;
    size_t numIndices;

    VertexBuffer vertexBuf;
    IndexBuffer indexBuf;
};
