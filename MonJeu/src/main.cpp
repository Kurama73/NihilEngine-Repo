// main.cpp
#include <iostream>
#include <NihilEngine/Window.h>
#include <NihilEngine/Renderer.h>
#include <NihilEngine/Camera.h>
#include <NihilEngine/Input.h>
#include <NihilEngine/EntityController.h>
#include <NihilEngine/Audio.h>
#include <NihilEngine/Physics.h>
#include <NihilEngine/TextureManager.h>
#include <NihilEngine/Environment.h>
#include <NihilEngine/Performance.h>
#include <MonJeu/VoxelWorld.h>
#include <MonJeu/Player.h>
#include <MonJeu/GameDebugOverlay.h>
#include <MonJeu/Constants.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

int main() {
    try {
        // === INITIALISATION ===
        std::cout << "[DEBUG] Creating window..." << std::endl;
        NihilEngine::Window window(1280, 720, "Mon Jeu (Powered by NihilEngine)");
        glfwFocusWindow(window.GetGLFWWindow());
        std::cout << "[DEBUG] Window created successfully" << std::endl;

        std::cout << "[DEBUG] Creating renderer..." << std::endl;
        NihilEngine::Renderer renderer;
        std::cout << "[DEBUG] Renderer created successfully" << std::endl;

        // Vérification des erreurs OpenGL après création du renderer
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cout << "[ERROR] OpenGL error after renderer creation: " << err << std::endl;
        }

        std::cout << "[DEBUG] Creating camera..." << std::endl;
        NihilEngine::Camera camera(60.0f, 1280.0f / 720.0f, 0.1f, 1000.0f);
        std::cout << "[DEBUG] Camera created successfully" << std::endl;

        std::cout << "[DEBUG] Initializing input..." << std::endl;
        NihilEngine::Input::Init(window.GetGLFWWindow());
        window.SetCamera(&camera);
        std::cout << "[DEBUG] Input initialized successfully" << std::endl;

        // === SYSTÈMES AVANCÉS ===

        // Audio
        std::cout << "[DEBUG] Initializing audio system..." << std::endl;
        if (!NihilEngine::AudioSystem::getInstance().initialize()) {
            std::cerr << "Echec de l'initialisation du système audio !" << std::endl;
            return -1;
        }
        std::cout << "[OK] Système audio initialise" << std::endl;

        MonJeu::VoxelWorld voxelWorld;
        MonJeu::Player myPlayer;
        MonJeu::Player testPlayer;
        MonJeu::GameDebugOverlay debugOverlay(1280, 720);

        // Contrôleur d'entités (switch avec TAB)
        NihilEngine::EntityController entityController(camera, GLFW_KEY_TAB);

        // Physique
        std::cout << "[DEBUG] Initializing physics world..." << std::endl;
        NihilEngine::PhysicsWorld* physicsWorld = new NihilEngine::PhysicsWorld();
        physicsWorld->initialize();
        std::cout << "[OK] Monde physique initialise" << std::endl;

        // Textures (Minecraft ou fallback)
        std::string minecraftPath = "MonJeu/assets/texturepack";
        if (NihilEngine::TextureManager::getInstance().loadMinecraftTexturePack(minecraftPath)) {
            std::cout << "[OK] Textures Minecraft chargees depuis: " << minecraftPath << std::endl;
        } else {
            std::cout << "[WARN] Echec du chargement des textures Minecraft." << std::endl;
            std::cout << "   Creation de textures de secours..." << std::endl;
            NihilEngine::TextureManager::getInstance().createFallbackTextures();
            std::cout << "[OK] Textures de secours prêtes" << std::endl;
        }

        // Atlas de textures
        std::cout << "[DEBUG] Creating texture atlas..." << std::endl;
        GLuint textureAtlas = 0;
        try {
            textureAtlas = NihilEngine::TextureManager::getInstance().createTextureAtlas();
            std::cout << "[DEBUG] Texture atlas created successfully" << std::endl;

            while ((err = glGetError()) != GL_NO_ERROR) {
                std::cout << "[ERROR] OpenGL error after texture atlas: " << err << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "[ERROR] Exception during texture atlas creation: " << e.what() << std::endl;
            throw;
        }
        voxelWorld.SetTextureAtlas(textureAtlas);
        std::cout << "[DEBUG] Texture atlas assigned to VoxelWorld" << std::endl;

        // Environnement
        NihilEngine::Environment* environment = new NihilEngine::Environment();
        environment->setTimeOfDay(0.5f);
        environment->setWeather("clear", 1.0f);
        std::cout << "[OK] Système environnemental initialise" << std::endl;

        // Performance & LOD
        NihilEngine::PerformanceMonitor::getInstance();
        NihilEngine::LODManager lodManager;
        lodManager.enableVSync(true);
        std::cout << "[OK] Système de performance initialise" << std::endl;

        // === GÉNÉRATION DU MONDE (avec LOD) ===
        std::cout << "[DEBUG] Initializing LOD-based world generation..." << std::endl;
        // Le système LOD générera automatiquement les chunks proches lors des premières mises à jour
        std::cout << "[DEBUG] LOD world generation initialized" << std::endl;

        // === SPAWN DU JOUEUR ===
        // Modifié : Spawn basé sur la hauteur du générateur, pas un raycast
        std::cout << "[DEBUG] Spawning player..." << std::endl;
        NihilEngine::TerrainGenerator& terrainGen = voxelWorld.GetProceduralGenerator().getTerrainGenerator();
        float spawnHeight = terrainGen.getHeight(0.5f, 0.5f); // Centre du bloc (0,0)
        myPlayer.SetPosition(glm::vec3(0.5f, spawnHeight + MonJeu::Constants::PLAYER_HEIGHT, 0.5f));
        std::cout << "[DEBUG] Player spawned at (0.5, " << spawnHeight + MonJeu::Constants::PLAYER_HEIGHT << ", 0.5)" << std::endl;


        testPlayer.SetPosition(myPlayer.GetPosition() + glm::vec3(2.0f, 0.0f, 2.0f));
        testPlayer.SetShowFOV(true);
        testPlayer.SetFacing(glm::vec3(1.0f, 0.0f, 0.0f));
        testPlayer.SetYaw(0.0f);
        testPlayer.SetPitch(0.0f);
        camera.SetPosition(myPlayer.GetPosition() + glm::vec3(0.0f, MonJeu::Constants::EYE_HEIGHT, 3.0f));

        entityController.AddControllableEntity(&myPlayer);
        entityController.AddControllableEntity(&testPlayer);

        // === BOUCLE PRINCIPALE ===
        std::cout << "[DEBUG] Starting main loop..." << std::endl;
        float lastTime = static_cast<float>(glfwGetTime());
        float fpsUpdateTimer = 0.0f;
        int frameCount = 0;

        while (!window.ShouldClose()) {
            float currentTime = static_cast<float>(glfwGetTime());
            float deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            frameCount++;
            fpsUpdateTimer += deltaTime;
            float fps = fpsUpdateTimer > 0.0f ? frameCount / fpsUpdateTimer : 0.0f;
            if (fpsUpdateTimer >= 1.0f) {
                fpsUpdateTimer = 0.0f;
                frameCount = 0;
            }

            // === MISE À JOUR ===
            NihilEngine::PerformanceMonitor::getInstance().startFrame();
            physicsWorld->update(deltaTime);
            environment->updateAtmosphere(deltaTime);
            environment->updateWeather(deltaTime);
            environment->updateLights(deltaTime);

            NihilEngine::AudioSystem::getInstance().setListenerPosition(camera.GetPosition());
            NihilEngine::AudioSystem::getInstance().setListenerOrientation(camera.GetForward(), camera.GetUp());
            NihilEngine::AudioSystem::getInstance().update();

            // Input
            if (NihilEngine::Input::IsKeyTriggered(GLFW_KEY_ESCAPE)) {
                glfwSetWindowShouldClose(window.GetGLFWWindow(), GLFW_TRUE);
            }

            if (NihilEngine::Input::IsKeyTriggered(GLFW_KEY_F3)) debugOverlay.ToggleDebugInfo();
            if (NihilEngine::Input::IsKeyTriggered(GLFW_KEY_F5)) debugOverlay.ToggleFPS();
            if (NihilEngine::Input::IsKeyTriggered(GLFW_KEY_F6)) debugOverlay.TogglePositions();
            if (NihilEngine::Input::IsKeyTriggered(GLFW_KEY_F7)) debugOverlay.ToggleChunkInfo();
            if (NihilEngine::Input::IsKeyTriggered(GLFW_KEY_F4)) myPlayer.ToggleRaycastVis();

            entityController.Update(deltaTime, voxelWorld);
            voxelWorld.UpdateDirtyChunks();

            // Mise à jour du système LOD
            voxelWorld.UpdateLOD(camera.GetPosition(), deltaTime);

            // Interaction
            glm::vec3 origin = camera.GetPosition();
            glm::vec3 direction = camera.GetForward();
            if (NihilEngine::Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
                NihilEngine::RaycastHit hit;
                if (voxelWorld.Raycast(origin, direction, MonJeu::Constants::RAYCAST_DISTANCE, hit)) {
                    voxelWorld.SetVoxelActive(hit.blockPosition.x, hit.blockPosition.y, hit.blockPosition.z, false);
                }
            }
            if (NihilEngine::Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
                NihilEngine::RaycastHit hit;
                if (voxelWorld.Raycast(origin, direction, MonJeu::Constants::RAYCAST_DISTANCE, hit)) {
                    glm::ivec3 placePos = hit.blockPosition + glm::ivec3(hit.hitNormal.x, hit.hitNormal.y, hit.hitNormal.z);
                    voxelWorld.SetVoxelActive(placePos.x, placePos.y, placePos.z, true);
                }
            }

            // === RENDU ===
            renderer.Clear();
            renderer.EnableFog(false);

            auto envEffects = environment->getEffects();
            bool hasFog = !envEffects.empty();
            renderer.EnableFog(hasFog);
            if (hasFog) {
                for (const auto& effect : envEffects) {
                    if (effect.type == "fog") {
                        renderer.SetFogColor(effect.color);
                        renderer.SetFogDensity(effect.density);
                        break;
                    }
                }
            }

            renderer.UpdateParticles(deltaTime);
            voxelWorld.Render(renderer, camera);
            entityController.Render(renderer, camera, true);
            if (myPlayer.IsRaycastVisible()) {
                myPlayer.RenderRaycast(renderer, camera, voxelWorld);
            }

            glDisable(GL_DEPTH_TEST);
            renderer.DrawCrosshair(window.GetWidth(), window.GetHeight());
            glEnable(GL_DEPTH_TEST);

            debugOverlay.RenderDebugInfo(
                fps,
                static_cast<int>(voxelWorld.GetChunks().size()),
                camera.GetPosition(),
                myPlayer.GetPosition()
            );

            debugOverlay.AddText("--- SYSTEMES AVANCES ---", 10, 120);
            debugOverlay.AddText("Audio: OK", 10, 140);
            debugOverlay.AddText("Physique: OK", 10, 160);
            debugOverlay.AddText("Performance: " + std::to_string((int)NihilEngine::PerformanceMonitor::getInstance().getFPS()) + " FPS", 10, 180);
            debugOverlay.AddText("Frame Time: " + std::to_string(NihilEngine::PerformanceMonitor::getInstance().getFrameTime() * 1000.0f) + " ms", 10, 200);
            debugOverlay.AddText("LOD System: ACTIVE", 10, 220);
            debugOverlay.AddText("Chunks: " + std::to_string(voxelWorld.GetChunkCount()), 10, 240);

            NihilEngine::PerformanceMonitor::getInstance().endFrame();

            NihilEngine::Input::Update();
            window.OnUpdate();
        }

        // === NETTOYAGE ===
        NihilEngine::AudioSystem::getInstance().shutdown();
        delete physicsWorld;
        delete environment;
        NihilEngine::Input::Shutdown();

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        NihilEngine::Input::Shutdown();
        return 1;
    }
}