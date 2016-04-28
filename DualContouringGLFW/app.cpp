#include "app.hpp"

#include "ShaderLoader.hpp"
#include "Mesh.hpp"
#include "Octree.hpp"

#include "GenerateTask.hpp"

vec3 DrawQueueSort::camPos = vec3(0.f);

std::priority_queue<Surface*, std::deque<Surface*>, DrawQueueSort> GetDrawQueue(std::array< std::array< std::unique_ptr<Surface>, 3>, 3>& objects)
{
    std::priority_queue<Surface*, std::deque<Surface*>, DrawQueueSort> drawQueue;
    for (auto& arr : objects)
    {
        for (auto& surface : arr)
        {
            drawQueue.push(surface.get());
        }
    }
    return drawQueue;
}

App::App()
    : running(true)
    , optionToggleGracePeriod(0.2f)
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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

    // Initialise some OpenGL wizardry
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)(sizeof(glm::vec3) * 1));

    // Create our Surface objects
    for (size_t y = 0; y < surfaces.size(); y++)
    {
        for (size_t x = 0; x < surfaces[y].size(); x++)
        {
            surfaces[y][x] = std::unique_ptr<Surface>(new Surface);
            surfaces[y][x]->position = vec3((static_cast<int>(x)-1)*OctreeSize, 0.f, (static_cast<int>(y)-1)*OctreeSize);
        }
    }

    // Workout how many threads we can use
    numThreads = std::thread::hardware_concurrency();
    // If we can, reserve 2 for the main thread and the render thread
    if (numThreads > 2) numThreads -= 2;

    farm = std::make_unique<ThreadFarm>();

    glfwMakeContextCurrent(NULL); // Give up the context

    return 0;
}

int App::Run()
{    
    // Start the render thread
    renderThread = pThread(new std::thread(std::mem_fun(&App::RenderThread), this));

    // Start generating surfaces
    const std::vector<float> Thresholds{ -1.f, 0.1f, 1.f, 10.f, 50.f };
    int currentThresholdIndex = 0;
    //GenerateSurfaces(Thresholds[currentThresholdIndex]);
    regenerateSurfaces = true;

    prev = clock.now();
    while (true)
    {
        auto now = clock.now();
        delta = now - prev;
        prev = now;

        runTime += delta.count();

        outputMutex.lock();
        std::string title = "DualContouring FPS: " + std::to_string(1.f / delta.count());
        outputMutex.unlock();
        glfwSetWindowTitle(window, title.c_str());

        // Process inputs and events
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS
         || glfwWindowShouldClose(window)) // May be unable to call glfwWindowShouldClose from here
        {
            running = false; // End RenderThread
            break;
        }
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        {
            if (runTime > wireframeModeTime + optionToggleGracePeriod)
            {
                wireframeModeMutex.lock();
                wireframeMode = !wireframeMode;
                wireframeModeTime = static_cast<float>(runTime);
                outputMutex.lock();
                std::cout << "Wireframe mode " << ((wireframeMode) ? "enabled" : "disabled") << std::endl;
                outputMutex.unlock();
                wireframeModeMutex.unlock();
            }
        }
        if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
        {
            if (runTime > regenerateTime + optionToggleGracePeriod)
            {
                regenerateSurfaces = true;
                regenerateTime = static_cast<float>(runTime);
            }
        }

        if (regenerateSurfaces == true && regeneratingSurfaces == false)
        {
            regeneratingSurfaces = true;
            outputMutex.lock();
            std::cout << "\nGenerating Meshes with threshold: " << Thresholds[currentThresholdIndex] << std::endl;
            outputMutex.unlock();
            std::thread genThread([&]{ GenerateSurfaces(Thresholds[currentThresholdIndex]); });
            currentThresholdIndex = (currentThresholdIndex + 1) % static_cast<int>(Thresholds.size());
            genThread.detach(); // Continue on and don't hang the main thread
            regenerateSurfaces = false;
        }
    }
    renderThread->join();
    glfwTerminate();
    return EXIT_SUCCESS;
}

void App::RenderThread()
{
    glfwMakeContextCurrent(window); // Call this here, so this thread can use glfw
    glfwSwapInterval(0); // Setting this to 1 should enable vsync, but doesn't (possibly a windows issue)

    static float rotateX = DegToRad(-45.f), rotateY = 0.f;
    float distance = static_cast<float>(OctreeSize)*2.f;

    while (running)
    {
        // Workout where the camera is
        rotateY += DegToRad(15.f)*delta.count();
        glm::vec3 dir(0.f, 0.f, 1.f);

        dir = glm::rotateX(dir, rotateX);
        dir = glm::rotateY(dir, rotateY);

        glm::vec3 position = dir * distance;
        position.y -= static_cast<float>(OctreeSize)*.25f;
        DrawQueueSort::camPos = position;

        // Get the draw queue
        auto drawQueue = GetDrawQueue(surfaces);

        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        wireframeModeMutex.lock();
        if (!wireframeMode)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        wireframeModeMutex.unlock();

        glm::mat4 projection = glm::perspective(90.f, static_cast<float>(WindowWidth) / static_cast<float>(WindowHeight), 0.1f, 500.f);
        glm::mat4 modelview = glm::lookAt(position - dir, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));

        glUseProgram(shaderProgramID);
        const GLint location = glGetUniformLocation(shaderProgramID, "MVP");
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(projection*modelview));

        // Draw each Surface
        while (!drawQueue.empty())
        {
            DrawSurface(drawQueue.top());
            drawQueue.pop();
        }

        glUseProgram(0);
        glfwSwapBuffers(window);
    }
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    // Destroy the surfaces manually here
    for (auto& arr : surfaces)
    {
        for (auto& surface : arr)
        {            
            surface.reset(nullptr);
        }
    }
}

void App::DrawSurface(Surface *surface)
{
    auto mesh = surface->mesh.get();
    if (mesh != nullptr)
    {
        mesh->Bind();

        glDrawElements(GL_TRIANGLES, static_cast<int>(mesh->numIndices), GL_UNSIGNED_INT, (void*)0);

        glBindVertexArray(0);
    }
}

void App::GenerateSurfaces(float Threshold)
{
    // Create a GenerateTask for each surface
    for (auto y = surfaces.begin(); y != surfaces.end(); y++)
    {
        for (auto x = y->begin(); x != y->end(); x++)
        {
            GenerateTask *task = new GenerateTask;
            GenerateTask::Args *args = new GenerateTask::Args;
            args->farm = farm.get();
            args->surface = x->get();
            args->threshold = Threshold;
            args->size = OctreeSize;
            args->min = (*x)->position + vec3(-OctreeSize*.5f);
            task->SetArgs(args);
            farm->PushNewTask(task);
        }
    }

    farm->Run(numThreads);
    regeneratingSurfaces = false;
}
