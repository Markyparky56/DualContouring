#include "Mesh.hpp"

void Mesh::Initialise()
{
    glGenVertexArrays(1, &vertexArrayObj);
    glGenBuffers(1, &vertexBuffer);
    glGenBuffers(1, &indexBuffer);

    glBindVertexArray(vertexArrayObj);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)(sizeof(glm::vec3) * 1));

    glBindVertexArray(0);
}

void Mesh::UploadData(const VertexBuffer & vertices, const IndexBuffer & indices)
{
    if (vertices.empty() || indices.empty())
    {
        return;
    }

    glBindVertexArray(vertexArrayObj);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), &indices[0], GL_STATIC_DRAW);
    numIndices = indices.size();

    printf("Mesh: %d vertices %d triangles\n", static_cast<int>(vertices.size()), static_cast<int>(numIndices) / 3);

    glBindVertexArray(0);
}

void Mesh::Destroy()
{
    glDeleteBuffers(1, &indexBuffer);
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vertexArrayObj);
}
