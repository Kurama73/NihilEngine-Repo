#include <NihilEngine/Window.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace NihilEngine {

    Window::Window(int width, int height, const char* title)
        : m_Width(width), m_Height(height) {
        // Initialiser GLFW
        if (!glfwInit()) {
            std::cerr << "Erreur: Init GLFW" << std::endl;
            return;
        }

        // Créer la fenêtre
        m_Window = glfwCreateWindow(width, height, title, NULL, NULL);
        if (!m_Window) {
            std::cerr << "Erreur: Création fenêtre GLFW" << std::endl;
            glfwTerminate();
            return;
        }

        // Définir le contexte OpenGL
        glfwMakeContextCurrent(m_Window);

        glfwSetWindowUserPointer(m_Window, this);

        glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);

        // Initialiser GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Erreur: Init GLAD" << std::endl;
            return;
        }

        glViewport(0, 0, width, height);

        std::cout << "NihilEngine initialised." << std::endl;
        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    }

    Window::~Window() {
        glfwDestroyWindow(m_Window);
        glfwTerminate();
        std::cout << "NihilEngine stopped." << std::endl;
    }

    void Window::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
    {
        // 1. Récupérer notre instance de 'Window'
        Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (!win) return;

        // 2. Mettre à jour le viewport OpenGL
        glViewport(0, 0, width, height);

        // 3. Mettre à jour la taille dans notre classe
        win->m_Width = width;
        win->m_Height = height;

        // 4. Mettre à jour l'aspect ratio de la caméra !
        if (win->m_Camera) {
            // Éviter la division par zéro si la fenêtre est minimisée
            if (height > 0) {
                win->m_Camera->SetAspect((float)width / (float)height);
            }
        }
    }

    bool Window::ShouldClose() {
        return glfwWindowShouldClose(m_Window);
    }

    void Window::OnUpdate() {
        // Gérer les événements (clavier, souris...)
        glfwPollEvents();
        // Échanger les buffers (afficher ce qui a été dessiné)
        glfwSwapBuffers(m_Window);
    }
}