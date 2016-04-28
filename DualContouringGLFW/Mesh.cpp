#include "Mesh.hpp"
#include <glfw3.h>
#include <iostream>
#include "ThreadTask.hpp" // include for outputMutex

void Mesh::Initialise()
{
    //glGenVertexArrays(1, &vertexArrayObj);
    //glGenBuffers(1, &vertexBuffer);
    //glGenBuffers(1, &indexBuffer);

    //glBindVertexArray(vertexArrayObj);
    //glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);    

    //glBindVertexArray(0);
}

void Mesh::UploadData(const VertexBuffer & vertices, const IndexBuffer & indices)
{
    if (vertices.empty() || indices.empty())
    {
        return;
    }

    vertexBuf = vertices;
    indexBuf = indices;

    numIndices = indices.size();
    
    outputMutex.lock();
    std::cout << "Mesh generated with " << static_cast<int>(vertices.size()) << " and " << static_cast<int>(numIndices) / 3 << " triangles" << std::endl;
    outputMutex.unlock();
}

void Mesh::Bind()
{
    if (!buffersGenerated)
    {
        glGenVertexArrays(1, &vertexArrayObj);
        glGenBuffers(1, &vertexBuffer);
        glGenBuffers(1, &indexBuffer);
        buffersGenerated = true;
    }

    glBindVertexArray(vertexArrayObj);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVertex) * vertexBuf.size(), vertexBuf.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indexBuf.size(), indexBuf.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)(sizeof(glm::vec3) * 1));
}

void Mesh::Destroy()
{
    glDeleteBuffers(1, &indexBuffer);
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vertexArrayObj);
}
