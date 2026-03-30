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

        // Request a core profile context compatible with OpenGL compute shaders.
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 4);
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

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

        glGetIntegerv(GL_MAJOR_VERSION, &m_GLVersionMajor);
        glGetIntegerv(GL_MINOR_VERSION, &m_GLVersionMinor);
        m_GpuComputeSupported = (m_GLVersionMajor > 4) || (m_GLVersionMajor == 4 && m_GLVersionMinor >= 3);

        if (m_GpuComputeSupported) {
            GLint maxCount[3] = {0, 0, 0};
            GLint maxSize[3] = {0, 0, 0};
            GLint maxInvocations = 0;

            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxCount[0]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &maxCount[1]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &maxCount[2]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxSize[0]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &maxSize[1]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &maxSize[2]);
            glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxInvocations);

            std::cout << "[Window] GPU compute available (OpenGL " << m_GLVersionMajor << "." << m_GLVersionMinor << ")" << std::endl;
            std::cout << "[Window] Max compute work group count: "
                      << maxCount[0] << ", " << maxCount[1] << ", " << maxCount[2] << std::endl;
            std::cout << "[Window] Max compute work group size: "
                      << maxSize[0] << ", " << maxSize[1] << ", " << maxSize[2] << std::endl;
            std::cout << "[Window] Max compute invocations: " << maxInvocations << std::endl;
        } else {
            std::cout << "[Window WARNING] OpenGL 4.3+ not available. Falling back to CPU generation path." << std::endl;
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