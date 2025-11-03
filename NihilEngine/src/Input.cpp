#include <NihilEngine/Input.h>

namespace NihilEngine {

    // (Initialisation des membres statiques... reste la même)
    GLFWwindow* Input::s_Window = nullptr;
    double Input::s_LastMouseX = 0.0;
    double Input::s_LastMouseY = 0.0;
    double Input::s_MouseDeltaX = 0.0;
    double Input::s_MouseDeltaY = 0.0;
    bool Input::s_FirstMouse = true;
    std::map<int, bool> Input::s_CurrentMouseButtons;
    std::map<int, bool> Input::s_LastMouseButtons;
    std::map<int, bool> Input::s_CurrentKeys;
    std::map<int, bool> Input::s_LastKeys;


    void Input::Init(GLFWwindow* window) {
        s_Window = window;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        glfwSetCursorPosCallback(window, CursorPosCallback);
        glfwSetKeyCallback(window, KeyCallback);
        glfwSetMouseButtonCallback(window, MouseButtonCallback);
    }

    void Input::Update() {
        // Copie l'état actuel dans "dernier état"
        s_LastMouseButtons = s_CurrentMouseButtons;
        s_LastKeys = s_CurrentKeys;

        // Réinitialise les deltas (important!)
        s_MouseDeltaX = 0.0;
        s_MouseDeltaY = 0.0;

        // --- LA CORRECTION EST ICI ---
        // Cet appel est déjà fait par Window::OnUpdate()
        // glfwPollEvents(); // <-- SUPPRIMER CETTE LIGNE
    }

    bool Input::IsKeyPressed(int key) {
        return glfwGetKey(s_Window, key) == GLFW_PRESS;
    }

    bool Input::IsKeyTriggered(int key) {
        return s_CurrentKeys[key] && !s_LastKeys[key];
    }

    bool Input::IsMouseButtonPressed(int button) {
        return s_CurrentMouseButtons[button] && !s_LastMouseButtons[button];
    }

    bool Input::IsMouseButtonHeld(int button) {
        return s_CurrentMouseButtons[button];
    }

    void Input::GetMouseDelta(double& x, double& y) {
        x = s_MouseDeltaX;
        y = s_MouseDeltaY;
    }

    void Input::GetMousePos(double& x, double& y) {
        glfwGetCursorPos(s_Window, &x, &y);
    }

    // Callbacks
    void Input::CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
        if (s_FirstMouse) {
            s_LastMouseX = xpos;
            s_LastMouseY = ypos;
            s_FirstMouse = false;
        }

        s_MouseDeltaX = xpos - s_LastMouseX;
        s_MouseDeltaY = s_LastMouseY - ypos; // Inverted Y
        s_LastMouseX = xpos;
        s_LastMouseY = ypos;
    }

    void Input::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS) {
            s_CurrentKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            s_CurrentKeys[key] = false;
        }
    }

    void Input::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        if (action == GLFW_PRESS) {
            s_CurrentMouseButtons[button] = true;
        } else if (action == GLFW_RELEASE) {
            s_CurrentMouseButtons[button] = false;
        }
    }
}