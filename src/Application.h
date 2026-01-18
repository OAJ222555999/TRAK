#pragma once
#include "Renderer.h"


struct GLFWwindow;

class Application
{
public:
    Application();
    ~Application();

    void run();
    Renderer& getRenderer() { return m_Renderer; }


private:
    void initWindow();
    void shutdown();

    GLFWwindow* m_Window;
    bool m_Running;
    Renderer m_Renderer;

};
