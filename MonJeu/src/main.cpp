// src/main.cpp
#include <MonJeu/Game.h>
#include <NihilEngine/Input.h> // Pour le Shutdown statique
#include <iostream>

int main() {
    try {
        // Crée l'objet principal du jeu
        MonJeu::Game game;

        // Lance la boucle principale
        game.Run();

    } catch (const std::exception& e) {
        // Gestion globale des erreurs
        std::cerr << "ERREUR FATALE: " << e.what() << std::endl;
        NihilEngine::Input::Shutdown(); // Nettoyage des systèmes statiques
        return 1;
    }

    return 0;
}