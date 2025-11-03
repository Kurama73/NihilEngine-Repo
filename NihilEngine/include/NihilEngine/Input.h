#pragma once

#include <GLFW/glfw3.h>
#include <map> // Nécessaire pour suivre l'état des boutons

namespace NihilEngine {
    class Input {
    public:
        static void Init(GLFWwindow* window);
        static void Update(); // <-- NOUVEAU: À appeler à la fin de la boucle principale

        static bool IsKeyPressed(int key); // Reste "maintenu"
        static bool IsKeyTriggered(int key); // <-- OPTIONNEL: Si tu veux le même effet pour le clavier

        // --- NOUVEAU: Gestion de la souris ---
        static bool IsMouseButtonPressed(int button); // Vrai 1 seule image (au moment du clic)
        static bool IsMouseButtonHeld(int button);    // Vrai tant que c'est maintenu
        // ------------------------------------

        static void GetMouseDelta(double& x, double& y);
        static void GetMousePos(double& x, double& y);

    private:
        static GLFWwindow* s_Window;

        // Pour la souris
        static double s_LastMouseX, s_LastMouseY;
        static double s_MouseDeltaX, s_MouseDeltaY;
        static bool s_FirstMouse;

        // --- NOUVEAU: Suivi d'état des boutons ---
        static std::map<int, bool> s_CurrentMouseButtons;
        static std::map<int, bool> s_LastMouseButtons;
        static std::map<int, bool> s_CurrentKeys;
        static std::map<int, bool> s_LastKeys;
        // -----------------------------------------

        // Callbacks
        static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
        static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods); // <-- NOUVEAU
    };
}