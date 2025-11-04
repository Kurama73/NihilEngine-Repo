#pragma once

#include <GLFW/glfw3.h>
#include <map>
#include <functional>

namespace NihilEngine {
    class Input {
    public:
        static void Init(GLFWwindow* window);
        static void Update();
        static void Shutdown();

        static bool IsKeyPressed(int key);
        static bool IsKeyTriggered(int key);
        static bool IsMouseButtonPressed(int button);
        static bool IsMouseButtonHeld(int button);
        static bool IsKeyDown(int key);

        static void GetMouseDelta(double& x, double& y);
        static void GetMousePos(double& x, double& y);

        // Action mapping
        static void BindKeyAction(int key, std::function<void()> action);
        static void ProcessActions();

    private:
        static GLFWwindow* s_Window;
        static double s_LastMouseX, s_LastMouseY;
        static double s_MouseDeltaX, s_MouseDeltaY;
        static bool s_FirstMouse;

        static std::map<int, bool> s_CurrentMouseButtons;
        static std::map<int, bool> s_LastMouseButtons;
        static std::map<int, bool> s_CurrentKeys;
        static std::map<int, bool> s_LastKeys;

        static std::map<int, std::function<void()>> s_KeyActions;

        static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
        static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    };
}