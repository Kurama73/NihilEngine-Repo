#include <NihilEngine/Window.h>
#include <NihilEngine/Constants.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <NihilEngine/Input.h>
#include <iostream>

namespace NihilEngine {

    Window::Window(int width, int height, const char* title, bool fullscreen)
        : m_Width(width), m_Height(height), m_Fullscreen(fullscreen) {
        std::cout << "[Window] Initializing GLFW..." << std::endl;
        if (!glfwInit()) {
            std::cout << "[Window ERROR] Failed to initialize GLFW" << std::endl;
            return;
        }
        std::cout << "[Window] GLFW initialized successfully" << std::endl;

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        if (fullscreen) {
            m_Window = glfwCreateWindow(mode->width, mode->height, title, monitor, NULL);
        } else {
            m_Window = glfwCreateWindow(width, height, title, NULL, NULL);
        }

        if (!m_Window) {
            std::cout << "[Window ERROR] Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return;
        }
        std::cout << "[Window] GLFW window created successfully" << std::endl;

        glfwMakeContextCurrent(m_Window);
        glfwSetWindowUserPointer(m_Window, this);
        glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout << "[Window ERROR] Failed to initialize GLAD" << std::endl;
            return;
        }
        std::cout << "[Window] GLAD initialized successfully" << std::endl;
        std::cout << "[Window] OpenGL version: " << glGetString(GL_VERSION) << std::endl;

        glViewport(0, 0, width, height);
    }

    Window::~Window() {
        NihilEngine::Input::Shutdown();

        if (m_Window) {
            glfwDestroyWindow(m_Window);
        }

        glfwTerminate();
    }

    bool Window::ShouldClose() {
        return glfwWindowShouldClose(m_Window);
    }

    void Window::OnUpdate() {
        glfwPollEvents();
        glfwSwapBuffers(m_Window);
    }

    void Window::SetFullscreen(bool fullscreen) {
        m_Fullscreen = fullscreen;
    }

    void Window::SetVSync(bool enabled) {
        glfwSwapInterval(enabled ? Constants::VSYNC_ENABLED : Constants::VSYNC_DISABLED);
    }

    void Window::FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
        Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
        glViewport(0, 0, width, height);
        win->m_Width = width;
        win->m_Height = height;
        if (win->m_Camera && height > 0) {
            win->m_Camera->SetAspect(static_cast<float>(width) / static_cast<float>(height));
        }
    }

}