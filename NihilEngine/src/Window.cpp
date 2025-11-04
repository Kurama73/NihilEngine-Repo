#include <NihilEngine/Window.h>
#include <NihilEngine/Constants.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <NihilEngine/Input.h>
#include <iostream>

namespace NihilEngine {

    Window::Window(int width, int height, const char* title, bool fullscreen)
        : m_Width(width), m_Height(height), m_Fullscreen(fullscreen) {
        if (!glfwInit()) {
            std::cerr << "Erreur: Init GLFW" << std::endl;
            return;
        }

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        if (fullscreen) {
            m_Window = glfwCreateWindow(mode->width, mode->height, title, monitor, NULL);
        } else {
            m_Window = glfwCreateWindow(width, height, title, NULL, NULL);
        }

        if (!m_Window) {
            std::cerr << "Erreur: Création fenêtre GLFW" << std::endl;
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(m_Window);
        glfwSetWindowUserPointer(m_Window, this);
        glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Erreur: Init GLAD" << std::endl;
            return;
        }

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