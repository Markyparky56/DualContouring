#include <GL\glew.h>
#include <glfw3.h>
#include <glm\glm.hpp>
#include <glm\ext.hpp>
#include <chrono>

#include "ShaderLoader.hpp"
#include "Mesh.hpp"
#include "Octree.hpp"

#define DegToRad(deg)((glm::pi<float>() * deg)/180.0f)

using hiResClock = std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using std::chrono::duration_cast;
using fsec = std::chrono::duration<float>;

const int WindowWidth = 1280;
const int WindowHeight = 720;

void DrawFrame(GLuint program, Mesh& mesh, const glm::vec3& pos, const glm::vec3& fwd, bool drawWireframe)
{
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    glm::mat4 projection = glm::perspective(90.f, static_cast<float>(WindowWidth) / static_cast<float>(WindowHeight), 0.1f, 500.f);
    glm::mat4 modelview = glm::lookAt(pos + fwd, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));

    glUseProgram(program);
    const GLint location = glGetUniformLocation(program, "MVP");
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(projection*modelview));

    if (!drawWireframe)
    {
        glPolygonMode(GL_FRONT, GL_FILL);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    glBindVertexArray(mesh.vertexArrayObj);
    glDrawElements(GL_TRIANGLES, static_cast<int>(mesh.numIndices), GL_UNSIGNED_INT, (void*)0);

    glUseProgram(0);
}

int main()
{
    hiResClock clock;

    if (!glfwInit())
    {
        printf("Failed to initialise GLFW\n");
        return EXIT_FAILURE;
    }

    //glfwWindowHint(GLFW_SAMPLES, 4);
    ////glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    ////glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WindowWidth, WindowHeight, "DualContouring", nullptr, nullptr);
    if (window == nullptr)
    {
        printf("Failed to open GLFW window!");
        glfwTerminate();
        return EXIT_FAILURE;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    // Initalise GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initalise GLEW\n");
        return EXIT_FAILURE;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    printf("OpenGL version: %s\n", glGetString(GL_VERSION));
    printf("OpenGL shading version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    std::string vertexShader, fragmentShader;
    if (!LoadShaderFile("shader.vert", vertexShader))
    {
        printf("Failed to load vertex shader!\n");
        return EXIT_FAILURE;
    }
    if (!LoadShaderFile("shader.frag", fragmentShader))
    {
        printf("Failed to load fragment shader!\n");
        return EXIT_FAILURE;
    }
    GLuint programID = CompilerAndLinkProgram(vertexShader, fragmentShader);
    
    const std::vector<float> Thresholds{ -1.0f, 0.1f, 1.0f, 10.0f, 50.0f };
    int threshIndex = -1;

    Mesh mesh;
    mesh.Initialise();

    OctreeNode* root = nullptr;
    const int octreeSize = 64;

    float rotateX = DegToRad(-45.0f), rotateY = DegToRad(0.0f);
    float distance = 64.0f;
    bool wireframeMode = false;
    bool refreshMesh = true;

    bool running = true;
    auto prev = clock.now();
    while (running)
    {
        auto now = clock.now();
        fsec delta = now - prev;
        prev = now;
        std::string title = "DualContouring FPS: " + std::to_string(1.0f / delta.count());
        glfwSetWindowTitle(window, title.c_str());

        // Check Events
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS
         || glfwWindowShouldClose(window))
        {
            running = false;
        }
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        {
            wireframeMode = !wireframeMode;
            printf("Wireframe mode %s\n", (wireframeMode) ? "enabled" : "disabled");
        }
        if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
        {
            refreshMesh = true;
        }

        // Refresh the mesh if need be
        if (refreshMesh)
        {
            refreshMesh = false;
            threshIndex = (threshIndex + 1) % static_cast<int>(Thresholds.size());
            printf("Generating mesh with error threshold=%.1f...\n", Thresholds[threshIndex]);
            auto start = clock.now();

            VertexBuffer vertices;
            IndexBuffer indices;

            root = BuildOctree(glm::ivec3(-octreeSize / 2), octreeSize, Thresholds[threshIndex]);
            GenerateMeshFromOctree(root, vertices, indices);
            mesh.UploadData(vertices, indices);

            auto end = clock.now();
            fsec timeTaken = end - start;
            printf("Time Taken: %.1fs\n", timeTaken.count());
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        rotateY += DegToRad(15.0f) * delta.count();

        glm::vec3 dir(0.0f, 0.0f, 1.0f);
        
        dir = glm::rotateX(dir, rotateX);
        dir = glm::rotateY(dir, rotateY);

        glm::vec3 position = dir * distance;

        DrawFrame(programID, mesh, position, -dir, wireframeMode);        
        glfwSwapBuffers(window);
    }

    DestroyOctree(root);
    mesh.Destroy();
    glfwTerminate();
    return EXIT_SUCCESS;
}