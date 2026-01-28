#include "Application.h"
#include <iostream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <filesystem>
#include <chrono>


static double lastX = 640.0;
static double lastY = 360.0;
static bool firstMouse = true;


Application::Application()
    : m_Window(nullptr), m_Running(true)
{
    initWindow();
}

Application::~Application()
{
    shutdown();
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    Application* app =
        static_cast<Application*>(glfwGetWindowUserPointer(window));

    if (!app) return;

    Camera* cam = app->getRenderer().getCamera();
    if (!cam) return;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float dx = static_cast<float>(xpos - lastX);
    float dy = static_cast<float>(lastY - ypos); // odwrócone Y

    lastX = xpos;
    lastY = ypos;

    cam->processMouse(dx, dy);
}


void Application::initWindow()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    m_Window = glfwCreateWindow(1280, 720, "Bokeh Renderer", nullptr, nullptr);
    if (!m_Window)
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_Window);

    glfwSetWindowUserPointer(m_Window, this);
    glfwSetCursorPosCallback(m_Window, mouse_callback);
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
{
    std::cerr << "Failed to initialize GLAD\n";
    return;
}



    std::cout << "Window created\n";
}

void Application::run()
{
    m_Renderer.init();
    
    std::cout << "Working directory: "
              << std::filesystem::current_path()
              << std::endl;

    auto lastTime = std::chrono::high_resolution_clock::now();


    while (!glfwWindowShouldClose(m_Window))
{
    auto now = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float>(now - lastTime).count();
    lastTime = now;

    

    Camera* cam = m_Renderer.getCamera();
if (cam)
{
    float forward = 0.0f;
    float right = 0.0f;

    if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS) forward += 1.0f;
    if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS) forward -= 1.0f;
    if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS) right += 1.0f;
    if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS) right -= 1.0f;

    cam->processKeyboard(forward, right, dt);
}

    BokehParams& p = m_Renderer.params;
    bool changed = false;
    if (glfwGetKey(m_Window, GLFW_KEY_UP) == GLFW_PRESS)    { p.focusDepth += 0.005f; changed = true; }
    if (glfwGetKey(m_Window, GLFW_KEY_DOWN) == GLFW_PRESS)  { p.focusDepth -= 0.005f; changed = true; }
    if (glfwGetKey(m_Window, GLFW_KEY_LEFT) == GLFW_PRESS)  { p.bokehRadius -= 0.1f; changed = true; }
    if (glfwGetKey(m_Window, GLFW_KEY_RIGHT) == GLFW_PRESS) { p.bokehRadius += 0.1f; changed = true; }
    if (glfwGetKey(m_Window, GLFW_KEY_Q) == GLFW_PRESS)     { p.aperture -= 0.05f; changed = true; }
    if (glfwGetKey(m_Window, GLFW_KEY_E) == GLFW_PRESS)     { p.aperture += 0.05f; changed = true; }

    if (glfwGetKey(m_Window, GLFW_KEY_1) == GLFW_PRESS)
        m_Renderer.setDebugView(DebugView::Final);

    if (glfwGetKey(m_Window, GLFW_KEY_2) == GLFW_PRESS)
        m_Renderer.setDebugView(DebugView::SceneColor);

    if (glfwGetKey(m_Window, GLFW_KEY_3) == GLFW_PRESS)
        m_Renderer.setDebugView(DebugView::Highlight);

    if (glfwGetKey(m_Window, GLFW_KEY_4) == GLFW_PRESS)
        m_Renderer.setDebugView(DebugView::BokehOnly);

    if (glfwGetKey(m_Window, GLFW_KEY_5) == GLFW_PRESS)
        m_Renderer.setDebugView(DebugView::DOFOnly);

    
    if (p.focusDepth < 0.0f) p.focusDepth = 0.0f;
    if (p.focusDepth > 1.0f) p.focusDepth = 1.0f;
    if (p.bokehRadius < 1.0f) p.bokehRadius = 1.0f;
    if (p.bokehRadius > 20.0f) p.bokehRadius = 20.0f;
    if (p.aperture < 1.0f) p.aperture = 1.0f;
    if (p.aperture > 22.0f) p.aperture = 22.0f;

    if (changed) {
        static float lastFocus = -1, lastAperture = -1;
        if (p.focusDepth != lastFocus || p.aperture != lastAperture) {
            std::cout << "[PARAMS] focus=" << p.focusDepth << " aperture=f/" << p.aperture << " radius=" << p.bokehRadius << "\n";
            lastFocus = p.focusDepth; lastAperture = p.aperture;
        }
    }

    m_Renderer.render();
    glfwSwapBuffers(m_Window);
    glfwPollEvents();
}

}

void Application::shutdown()
{
    glfwDestroyWindow(m_Window);
    glfwTerminate();
    std::cout << "Application shutdown\n";
}
