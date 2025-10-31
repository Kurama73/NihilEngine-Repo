#include <NihilEngine/Window.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace NihilEngine {

    Window::Window(int width, int height, const char* title) {
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

        // Initialiser GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Erreur: Init GLAD" << std::endl;
            return;
        }

        std::cout << "Moteur NihilEngine initialisé." << std::endl;
        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    }

    Window::~Window() {
        glfwDestroyWindow(m_Window);
        glfwTerminate();
        std::cout << "Moteur NihilEngine arrêté." << std::endl;
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