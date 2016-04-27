#pragma once
#include <thread>
#include <GL\glew.h>
#include <glfw3.h>
#include <glm\glm.hpp>
#include <glm\ext.hpp>
#include <memory>

#include "Surface.hpp"

#define DegToRad(deg)((glm::pi<float>() * deg)/180.0f)

const int WindowWidth = 1280;
const int WindowHeight = 720;

// App class to encapsulate the functionality of the program
class App
{
public:
    App();
    ~App();

    int Init();
    int Run();

private:
    void RenderThread();

    std::array< std::array< std::unique_ptr<Surface>, 3>, 3> surfaces;

    GLFWwindow *window;
    GLuint shaderProgramID;

};
