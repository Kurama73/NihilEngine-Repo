#pragma once

#include <NihilEngine/Camera.h>

struct GLFWwindow;

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
        GLFWwindow* GetGLFWWindow() const { return m_Window; }

        void SetCamera(Camera* camera) { m_Camera = camera; }
        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }

    private:
        GLFWwindow* m_Window;

        Camera* m_Camera = nullptr; // Pointeur vers la caméra principale
        int m_Width;
        int m_Height;

        // Callback statique pour GLFW
        static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    };
}