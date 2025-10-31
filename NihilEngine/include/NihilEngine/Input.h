#pragma once

struct GLFWwindow;

namespace NihilEngine {
    class Input {
    public:
        static void Init(GLFWwindow* window);
        static bool IsKeyPressed(int key);
        static bool IsMouseButtonPressed(int button);
        static void GetMousePosition(double& x, double& y);
        static void GetMouseDelta(double& dx, double& dy);

    private:
        static GLFWwindow* s_Window;
        static double s_LastMouseX, s_LastMouseY;
        static bool s_FirstMouse;
    };
}