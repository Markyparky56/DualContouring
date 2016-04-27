#include "app.hpp"

#include "ShaderLoader.hpp"
#include "Mesh.hpp"
#include "Octree.hpp"

App::App()
{
}

App::~App()
{
}

int App::Init()
{
    // Initalise GLFW
    if (!glfwInit())
    {
        printf("Failed to initalise GLFW\n");
        return EXIT_FAILURE;
    }

    window = glfwCreateWindow(WindowWidth, WindowHeight, "DualContouring", nullptr, nullptr);
    if (window == nullptr)
    {
        printf("Failed to open GLFW window!");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

    // Initalise GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initalise GLEW\n");
        return EXIT_FAILURE;
    }

    // Simple window setup
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Output what graphics api we're dealing with
    printf("OpenGL version: %s\n", glGetString(GL_VERSION));
    printf("OpenGL shading version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Load and compile our very simple shaders
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
    shaderProgramID = CompilerAndLinkProgram(vertexShader, fragmentShader);

    // Create our Surface objects
    for (size_t y = 0; y < surfaces.size(); y++)
    {
        for (size_t x = 0; x < surfaces[y].size(); x++)
        {

        }
    }

    return 0;
}

int App::Run()
{
    return 0;
}

void App::RenderThread()
{
    glfwMakeContextCurrent(window); // Call this here, so this thread can use glfw
    glfwSwapInterval(0);
}
