#include <NihilEngine/Input.h>
#include <GLFW/glfw3.h>

namespace NihilEngine {

    GLFWwindow* Input::s_Window = nullptr;
    double Input::s_LastMouseX = 0.0;
    double Input::s_LastMouseY = 0.0;
    bool Input::s_FirstMouse = true;

    void Input::Init(GLFWwindow* window) {
        s_Window = window;
        glfwSetInputMode(s_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Capture mouse
    }

    bool Input::IsKeyPressed(int key) {
        return glfwGetKey(s_Window, key) == GLFW_PRESS;
    }

    bool Input::IsMouseButtonPressed(int button) {
        return glfwGetMouseButton(s_Window, button) == GLFW_PRESS;
    }

    void Input::GetMousePosition(double& x, double& y) {
        glfwGetCursorPos(s_Window, &x, &y);
    }

    void Input::GetMouseDelta(double& dx, double& dy) {
        double x, y;
        glfwGetCursorPos(s_Window, &x, &y);

        if (s_FirstMouse) {
            s_LastMouseX = x;
            s_LastMouseY = y;
            s_FirstMouse = false;
        }

        dx = x - s_LastMouseX;
        dy = s_LastMouseY - y; // Inversé pour Y

        s_LastMouseX = x;
        s_LastMouseY = y;
    }
}