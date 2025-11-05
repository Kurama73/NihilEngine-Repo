// include/MonJeu/Game.h
#pragma once

#include <NihilEngine/Window.h>
#include <NihilEngine/Renderer.h>
#include <NihilEngine/Camera.h>
#include <NihilEngine/EntityController.h>
#include <NihilEngine/Physics.h>
#include <NihilEngine/Environment.h>
#include <MonJeu/VoxelWorld.h>
#include <MonJeu/Player.h>
#include <MonJeu/GameDebugOverlay.h>
#include <MonJeu/SaveManager.h>
#include <memory>

namespace MonJeu {

    class Game {
    public:
        /**
         * @brief Constructeur. Initialise tous les systèmes.
         */
        Game();

        /**
         * @brief Destructeur. Gère le nettoyage.
         */
        ~Game();

        /**
         * @brief Lance la boucle de jeu principale.
         */
        void Run();

    private:
        /**
         * @brief Initialise tous les systèmes du moteur (Fenêtre, Rendu, Audio, etc.).
         */
        void InitializeEngineSystems();

        /**
         * @brief Initialise tous les objets de jeu (Monde, Joueur, UI).
         */
        void InitializeGameObjects();

        /**
         * @brief Gère les entrées utilisateur globales (ex: F3, ESC).
         */
        void ProcessInput(float deltaTime);

        /**
         * @brief Met à jour la logique de jeu (Physique, IA, LOD, Joueur).
         */
        void Update(float deltaTime);

        /**
         * @brief Exécute la passe de rendu.
         */
        void Render();

        /**
         * @brief Sauvegarde l'état du joueur
         */
        void SavePlayerState();

        // --- Systèmes Moteur ---
        std::unique_ptr<NihilEngine::Window> m_Window;
        std::unique_ptr<NihilEngine::Renderer> m_Renderer;
        std::unique_ptr<NihilEngine::PhysicsWorld> m_PhysicsWorld;
        std::unique_ptr<NihilEngine::Environment> m_Environment;
        NihilEngine::Camera m_Camera;

        // --- Système de Sauvegarde ---
        std::unique_ptr<SaveManager> m_SaveManager;
        std::unique_ptr<WorldSaveManager> m_WorldSaveManager;

        // --- Objets de Jeu ---
        std::unique_ptr<MonJeu::VoxelWorld> m_VoxelWorld;
        std::unique_ptr<MonJeu::Player> m_Player;
        std::unique_ptr<NihilEngine::EntityController> m_EntityController;
        std::unique_ptr<MonJeu::GameDebugOverlay> m_DebugOverlay;

        float m_LastTime = 0.0f;
        float m_FPS = 0.0f;
        float m_PlayerSaveTimer = 0.0f; // Timer pour la sauvegarde périodique du joueur
    };

} // namespace MonJeu