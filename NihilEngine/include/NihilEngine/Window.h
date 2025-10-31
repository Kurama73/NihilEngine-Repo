#pragma once
struct GLFWwindow; // Déclaration avancée pour ne pas inclure glfw3.h dans un header

namespace NihilEngine {
    class Window {
    public:
        Window(int width, int height, const char* title);
        ~Window();

        // Empêcher la copie de la fenêtre
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        bool ShouldClose();
        void OnUpdate();
        GLFWwindow* GetGLFWWindow() { return m_Window; }
    private:
        GLFWwindow* m_Window; // Pointeur vers la fenêtre GLFW
    };
}