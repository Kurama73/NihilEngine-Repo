// main.cpp
#include <NihilEngine/Window.h>
#include <NihilEngine/Renderer.h>
#include <NihilEngine/Camera.h>
#include <NihilEngine/Input.h>
#include <MonJeu/VoxelWorld.h>
#include <MonJeu/Player.h>
#include <MonJeu/GameDebugOverlay.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

int main() {
    try {
        // === INITIALISATION ===
        NihilEngine::Window window(1280, 720, "Mon Jeu (Powered by NihilEngine)");
        NihilEngine::Renderer renderer;
        NihilEngine::Camera camera(60.0f, 1280.0f / 720.0f, 0.1f, 1000.0f);
        NihilEngine::Input::Init(window.GetGLFWWindow());
        window.SetCamera(&camera);

        MonJeu::VoxelWorld voxelWorld;
        MonJeu::Player player;
        MonJeu::GameDebugOverlay debugOverlay(1280, 720);

        // === CHARGER LA POLICE (OBLIGATOIRE) ===
        bool fontLoaded = false;
        std::vector<std::string> fontPaths = {
            "C:/Windows/Fonts/arial.ttf",
            "C:/Windows/Fonts/cour.ttf",
            "C:/Windows/Fonts/verdana.ttf",
            "C:/Windows/Fonts/times.ttf"
        };

        for (const auto& fontPath : fontPaths) {
            if (debugOverlay.LoadFont(fontPath.c_str(), 24)) {
                fontLoaded = true;
                std::cout << "Successfully loaded font: " << fontPath << std::endl;
                break;
            } else {
                std::cout << "Failed to load font: " << fontPath << std::endl;
            }
        }

        if (!fontLoaded) {
            std::cerr << "All fonts failed to load! Text will be invisible." << std::endl;
        }

        // === GÉNÉRER LE MONDE ===
        for (int chunkX = -3; chunkX <= 3; ++chunkX) {
            for (int chunkZ = -3; chunkZ <= 3; ++chunkZ) {
                voxelWorld.GenerateChunk(chunkX, chunkZ);
            }
        }

        // === SPAWN JOUEUR AU-DESSUS DU SOL ===
        glm::vec3 rayOrigin = glm::vec3(0.0f, 100.0f, 0.0f);
        glm::vec3 rayDirection = glm::vec3(0.0f, -1.0f, 0.0f);
        NihilEngine::RaycastHit hit;
        if (voxelWorld.Raycast(rayOrigin, rayDirection, 200.0f, hit)) {
            player.SetPosition(hit.hitPoint + glm::vec3(0.0f, 0.9f, 0.0f)); // Base à hit, center à hit + 0.9
        } else {
            player.SetPosition(glm::vec3(0.0f, 10.0f, 0.0f));
        }
        camera.SetPosition(player.GetPosition() + glm::vec3(0.0f, 0.9f, 3.0f));

        // === BOUCLE PRINCIPALE ===
        float lastTime = static_cast<float>(glfwGetTime());
        float fpsUpdateTimer = 0.0f;
        int frameCount = 0;

        while (!window.ShouldClose()) {
            float currentTime = static_cast<float>(glfwGetTime());
            float deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            // FPS counter
            frameCount++;
            fpsUpdateTimer += deltaTime;
            float fps = frameCount / fpsUpdateTimer;
            if (fpsUpdateTimer >= 1.0f) {
                fpsUpdateTimer = 0.0f;
                frameCount = 0;
            }

            // === INPUT ===
            if (NihilEngine::Input::IsKeyTriggered(GLFW_KEY_ESCAPE)) {
                glfwSetWindowShouldClose(window.GetGLFWWindow(), GLFW_TRUE);
            }

            // Debug toggles
            if (NihilEngine::Input::IsKeyTriggered(GLFW_KEY_F3)) debugOverlay.ToggleDebugInfo();
            if (NihilEngine::Input::IsKeyTriggered(GLFW_KEY_F5)) debugOverlay.ToggleFPS();
            if (NihilEngine::Input::IsKeyTriggered(GLFW_KEY_F6)) debugOverlay.TogglePositions();
            if (NihilEngine::Input::IsKeyTriggered(GLFW_KEY_F7)) debugOverlay.ToggleChunkInfo();
            if (NihilEngine::Input::IsKeyTriggered(GLFW_KEY_F4)) player.ToggleRaycastVis();

            // === MISE À JOUR ===
            player.Update(deltaTime, camera, voxelWorld);
            voxelWorld.UpdateDirtyChunks();

            // === RAYCAST INTERACTION ===
            glm::vec3 origin = camera.GetPosition();
            glm::vec3 direction = camera.GetForward();
            if (NihilEngine::Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
                if (voxelWorld.Raycast(origin, direction, 6.0f, hit)) {
                    voxelWorld.SetVoxelActive(hit.blockPosition.x, hit.blockPosition.y, hit.blockPosition.z, false);
                }
            }
            if (NihilEngine::Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
                if (voxelWorld.Raycast(origin, direction, 6.0f, hit)) {
                    glm::ivec3 placePos = hit.blockPosition + glm::ivec3(hit.hitNormal.x, hit.hitNormal.y, hit.hitNormal.z);
                    voxelWorld.SetVoxelActive(placePos.x, placePos.y, placePos.z, true);
                }
            }

            // === RENDU ===
            renderer.Clear();

            // 3D World
            voxelWorld.Render(renderer, camera);
            player.Render(renderer, camera);
            if (player.IsRaycastVisible()) {
                player.RenderRaycast(renderer, camera, voxelWorld);
            }

            // UI
            glDisable(GL_DEPTH_TEST);
            renderer.DrawCrosshair(window.GetWidth(), window.GetHeight());
            glEnable(GL_DEPTH_TEST);

            // Debug Overlay
            debugOverlay.RenderDebugInfo(
                fps,
                static_cast<int>(voxelWorld.GetChunks().size()),
                camera.GetPosition(),
                player.GetPosition()
            );

            NihilEngine::Input::Update(); // Réinitialise l'input de la frame précédente
            window.OnUpdate();            // Récupère l'input pour la frame suivante
        }

        // Shutdown
        NihilEngine::Input::Shutdown();

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        NihilEngine::Input::Shutdown();
        return 1;
    }
}