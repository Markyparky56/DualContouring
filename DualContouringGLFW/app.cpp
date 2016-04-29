#include "app.hpp"

#include "ShaderLoader.hpp"
#include "Mesh.hpp"
#include "Octree.hpp"

#include "GenerateTask.hpp"

vec3 DrawQueueSort::camPos = vec3(0.f);
Channel<std::string> *ThreadFarm::outputChannel = nullptr;

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
    , wireframeMode(false)
{
}

App::~App()
{
}

int App::Init(const int InNumThreads)
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
            surfaces[y][x] = std::make_unique<Surface>();
            surfaces[y][x]->position = vec3((static_cast<int>(x)-1)*OctreeSize, 0.f, (static_cast<int>(y)-1)*OctreeSize);
        }
    }

    if (InNumThreads == -1 || InNumThreads < 1)
    {
        // Workout how many threads we can use
        numThreads = std::thread::hardware_concurrency();
        // If we can, reserve 2 for the main thread and the render thread
        if (numThreads > 2) numThreads -= 2;
    }
    else
    {
        numThreads = InNumThreads;
    }

    farm = std::make_unique<ThreadFarm>();
    outputChannel = std::make_unique<Channel<std::string>>();
    ThreadFarm::outputChannel = outputChannel.get();

    glfwMakeContextCurrent(NULL); // Give up the context

    return 0;
}

int App::Run(bool TestMode)
{    
    // Start the render thread
    renderThread = pThread(new std::thread(std::mem_fun(&App::RenderThread), this));

    // Start the output thread to pull from the outputChannel
    outputThread = pThread(new std::thread(std::mem_fun(&App::OutputThread), this));

    // Start generating surfaces
    const std::vector<float> Thresholds{ -1.f, 0.1f, 1.f, 10.f, 50.f };
    int nextThresholdIndex = 0, thresholdTestCount = 0;
    //GenerateSurfaces(Thresholds[currentThresholdIndex]);
    regenerateSurfaces = true;
    regeneratingSurfaces = false;

    if(TestMode)
    {
        std::stringstream ss;
        ss << "Beginning Test Sequence..." << std::endl;
        std::unique_lock<std::mutex> lock(outputMutex);
        outputChannel->Write(ss.str());
    }

    while (running)
    {
        //deltaMutex.lock();
        //std::string title = "DualContouring FPS: " + std::to_string(1.f / delta.count());
        //glfwSetWindowTitle(window, title.c_str());
        //deltaMutex.unlock();

        // Process inputs and events
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS
         || glfwWindowShouldClose(window) 
         || thresholdTestCount == static_cast<int>(Thresholds.size()) && !regeneratingSurfaces) // Only important during a test run
        {
            running = false; // End RenderThread
            outputChannel->Exit(); // Break out of the channel's wait loop
            continue;
        }
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !TestMode)
        {
            if (runTime > wireframeModeTime + optionToggleGracePeriod)
            {
                std::unique_lock<std::mutex> lock(wireframeModeMutex);
                wireframeMode = !wireframeMode;
                wireframeModeTime = static_cast<float>(runTime);
                {
                    std::stringstream ss;
                    ss << "Wireframe mode " << ((wireframeMode) ? "enabled" : "disabled") << std::endl;
                    std::unique_lock<std::mutex> lock(outputMutex);
                    outputChannel->Write(ss.str());
                }
            }
        }
        if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS && !TestMode)
        {
            if (runTime > regenerateTime + optionToggleGracePeriod)
            {
                regenerateSurfaces = true;
                regenerateTime = static_cast<float>(runTime);
            }
        }
        if (TestMode == true)
        {
            if (regeneratingSurfaces == false && thresholdTestCount < static_cast<int>(Thresholds.size()))
            {
                regenerateSurfaces = true;
                thresholdTestCount++;
            }
        }

        if (regenerateSurfaces == true && regeneratingSurfaces == false)
        {
            regeneratingSurfaces = true;
            {                
                std::stringstream ss;
                ss << "\nGenerating Meshes with threshold: " << Thresholds[nextThresholdIndex] << std::endl;
                std::unique_lock<std::mutex> lock(outputMutex);
                outputChannel->Write(ss.str());
            }
            std::thread genThread([&]{ GenerateSurfaces(Thresholds[nextThresholdIndex]); });
            nextThresholdIndex = (nextThresholdIndex + 1) % static_cast<int>(Thresholds.size());
            genThread.detach(); // Continue on and don't hang the main thread
            regenerateSurfaces = false;
        }
    }

    std::stringstream ss;
    ss << "\nTotal execution time: " << runTime;
    std::cout << ss.str();

    renderThread->join();
    outputThread->join();
    glfwTerminate();
    return EXIT_SUCCESS;
}

void App::OutputThread()
{
    while (running)
    {
        std::string output = outputChannel->Read();
        if (!output.empty())
        {
            std::cout << output;
        }
    }
}

void App::RenderThread()
{
    glfwMakeContextCurrent(window); // Call this here, so this thread can use glfw
    glfwSwapInterval(0); // Setting this to 1 should enable vsync, but doesn't (possibly a windows issue)

    static float rotateX = DegToRad(-45.f), rotateY = 0.f;
    float distance = static_cast<float>(OctreeSize)*2.f;
    prev = clock.now();

    while (running)
    {
        auto now = clock.now();
        deltaMutex.lock();
        delta = now - prev;  
        runTime += delta.count();
        deltaMutex.unlock();
        prev = now;

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
#if defined(_DEBUG) || defined(WITH_TIMINGS) // Task's don't need to output in release mode
            args->outputChannel = outputChannel.get();
#endif
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
