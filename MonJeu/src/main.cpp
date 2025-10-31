// Inclure la classe Fenêtre de NOTRE moteur
#include <NihilEngine/Window.h>

// Inclure glad est nécessaire ici pour glClearColor
#include <glad/glad.h>

int main()
{
    // 1. Le MOTEUR crée la fenêtre
    NihilEngine::Window window(1280, 720, "Mon Jeu (Powered by NihilEngine)");

    // 2. Le JEU contrôle la boucle principale
    while (!window.ShouldClose())
    {
        // --- LOGIQUE DU JEU ---
        // Effacer l'écran (avec la couleur du jeu)
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f); // Bleu foncé
        glClear(GL_COLOR_BUFFER_BIT);

        // --- FIN LOGIQUE DU JEU ---


        // 3. Le MOTEUR met à jour l'écran
        window.OnUpdate();
    }

    // La fenêtre est détruite automatiquement à la fin du scope
    return 0;
}