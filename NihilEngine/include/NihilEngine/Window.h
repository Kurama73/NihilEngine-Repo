#pragma once

#include <NihilEngine/Camera.h>

struct GLFWwindow;

namespace NihilEngine {
    class Window {
    public:
        Window(int width, int height, const char* title, bool fullscreen = false);
        ~Window();

        bool ShouldClose();
        void OnUpdate();
        GLFWwindow* GetGLFWWindow() const { return m_Window; }

        void SetCamera(Camera* camera) { m_Camera = camera; }
        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }

        void SetFullscreen(bool fullscreen);
        void SetVSync(bool enabled);

    private:
        GLFWwindow* m_Window;
        Camera* m_Camera = nullptr;
        int m_Width, m_Height;
        bool m_Fullscreen = false;

        static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    };
}