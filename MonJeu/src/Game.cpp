// src/Game.cpp
#include <MonJeu/Game.h>
#include <NihilEngine/Input.h>
#include <NihilEngine/Audio.h>
#include <NihilEngine/TextureManager.h>
#include <NihilEngine/Performance.h>
#include <MonJeu/Constants.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace MonJeu {

Game::Game()
    : m_Camera(60.0f, 1280.0f / 720.0f, 0.1f, 1000.0f) //
{
    InitializeEngineSystems();
    InitializeGameObjects();
}

Game::~Game() {
    // Le nettoyage est géré par les unique_ptr,
    // mais les systèmes statiques/singletons doivent être arrêtés.
    NihilEngine::AudioSystem::getInstance().shutdown(); //
    NihilEngine::Input::Shutdown(); //
    std::cout << "Game shutdown." << std::endl;
}

void Game::InitializeEngineSystems() {
    std::cout << "[Game] Initializing Engine Systems..." << std::endl;

    // Initialiser le système de sauvegarde
    m_SaveManager = std::make_unique<SaveManager>();

    // Créer ou charger un monde par défaut
    std::string defaultWorldName = "world1";
    if (!m_SaveManager->WorldExists(defaultWorldName)) {
        std::cout << "[Game] Création du monde par défaut..." << std::endl;
        m_WorldSaveManager = m_SaveManager->CreateWorld(defaultWorldName, "Monde Principal", 12345, "Monde généré automatiquement");
    } else {
        std::cout << "[Game] Chargement du monde existant..." << std::endl;
        m_WorldSaveManager = m_SaveManager->LoadWorld(defaultWorldName);
    }

    if (!m_WorldSaveManager) {
        throw std::runtime_error("Échec de l'initialisation du système de sauvegarde");
    }

    m_Window = std::make_unique<NihilEngine::Window>(1280, 720, "Mon Jeu (Powered by NihilEngine)"); //
    glfwFocusWindow(m_Window->GetGLFWWindow());

    m_Renderer = std::make_unique<NihilEngine::Renderer>(); //

    NihilEngine::Input::Init(m_Window->GetGLFWWindow()); //
    m_Window->SetCamera(&m_Camera);

    if (!NihilEngine::AudioSystem::getInstance().initialize()) { //
        throw std::runtime_error("Echec de l'initialisation du système audio !");
    }

    m_PhysicsWorld = std::make_unique<NihilEngine::PhysicsWorld>(); //
    m_PhysicsWorld->initialize();

    m_Environment = std::make_unique<NihilEngine::Environment>(); //
    m_Environment->setTimeOfDay(0.5f);

    NihilEngine::LODManager lodManager; //
    lodManager.enableVSync(true);

    std::cout << "[Game] Engine Systems OK." << std::endl;
}

void Game::InitializeGameObjects() {
    std::cout << "[Game] Initializing Game Objects..." << std::endl;

    // 1. Textures (obligatoire avant VoxelWorld)
    std::string minecraftPath = "MonJeu/assets/texturepack";
    if (!NihilEngine::TextureManager::getInstance().loadMinecraftTexturePack(minecraftPath)) { //
        std::cout << "[WARN] Echec du chargement des textures Minecraft." << std::endl; //
        NihilEngine::TextureManager::getInstance().createFallbackTextures(); //
    }
    GLuint textureAtlas = NihilEngine::TextureManager::getInstance().createTextureAtlas(); //

    // 2. Monde
    m_VoxelWorld = std::make_unique<MonJeu::VoxelWorld>(12345, m_PhysicsWorld.get(), m_WorldSaveManager.get()); // Passe le monde physique et le gestionnaire de sauvegarde
    m_VoxelWorld->SetTextureAtlas(textureAtlas); //
    m_VoxelWorld->SetTerrainGpuPreferred(false);
    std::cout << "[Game] Terrain generation default mode: CPU (press F7 to toggle GPU experimental mode)." << std::endl;

    // 3. Joueur
    m_Player = std::make_unique<MonJeu::Player>(); //

    // Générer d'abord les chunks autour du spawn à la distance de rendu complète.
    glm::vec3 tentativeSpawnPos = glm::vec3(0.5f, 0.0f, 0.5f);
    const int spawnRadiusChunks = m_VoxelWorld->GetRecommendedStartupGenerationRadiusChunks();
    std::cout << "[Game] Generating startup area around spawn (radius "
              << spawnRadiusChunks << " chunks)..." << std::endl;
    m_VoxelWorld->GenerateSpawnArea(tentativeSpawnPos, spawnRadiusChunks);
    std::cout << "[Game] Spawn area ready, placing player..." << std::endl;

    const int spawnWorldX = static_cast<int>(std::floor(tentativeSpawnPos.x));
    const int spawnWorldZ = static_cast<int>(std::floor(tentativeSpawnPos.z));
    int groundY = m_VoxelWorld->FindHighestSolidBlockY(spawnWorldX, spawnWorldZ);
    if (groundY < 0) {
        // Fallback de sécurité si la colonne n'a pas été trouvée en mémoire.
        NihilEngine::TerrainGenerator& terrainGen = m_VoxelWorld->GetProceduralGenerator().getTerrainGenerator();
        groundY = static_cast<int>(std::floor(terrainGen.getHeight(static_cast<float>(spawnWorldX), static_cast<float>(spawnWorldZ))));
    }

    auto findSafeSpawnY = [this](const glm::vec3& basePos, int maxLiftSteps = 48) {
        glm::vec3 candidate = basePos;
        NihilEngine::AABB playerBox;
        const float halfWidth = Constants::COLLISION_RADIUS;
        const float halfHeight = Constants::PLAYER_HEIGHT * 0.5f;

        for (int step = 0; step <= maxLiftSteps; ++step) {
            playerBox.min = candidate - glm::vec3(halfWidth, halfHeight, halfWidth);
            playerBox.max = candidate + glm::vec3(halfWidth, halfHeight, halfWidth);
            if (!m_VoxelWorld->CheckCollision(playerBox)) {
                return candidate.y;
            }
            candidate.y += 1.0f;
        }

        return basePos.y;
    };

    // m_Position du joueur est le centre de la hitbox: on place donc le centre
    // à (haut du bloc + demi-hauteur du joueur + marge).
    glm::vec3 testSpawnPos = glm::vec3(
        static_cast<float>(spawnWorldX) + 0.5f,
        static_cast<float>(groundY) + 1.0f + (Constants::PLAYER_HEIGHT * 0.5f) + 0.05f,
        static_cast<float>(spawnWorldZ) + 0.5f
    );
    testSpawnPos.y = findSafeSpawnY(testSpawnPos);
    std::cout << "[Game] Safe spawn position resolved at y=" << testSpawnPos.y << std::endl;

    glm::vec3 spawnPos = testSpawnPos; //
    m_Player->SetPosition(spawnPos);
    m_Camera.SetPosition(spawnPos + glm::vec3(0.0f, Constants::EYE_HEIGHT, 0.0f)); //

    // Orienter la caméra vers -Z (devant le joueur) au lieu de +Z
    m_Camera.SetRotation(270.0f, 0.0f); // 270 degrés = -Z direction

    // Charger l'état du joueur s'il existe
    MonJeu::PlayerState playerState;
    if (m_WorldSaveManager->LoadPlayerState(playerState)) {
        // Pré-génère autour de la position sauvegardée pour éviter une chute dans le vide.
        m_VoxelWorld->GenerateSpawnArea(playerState.position, spawnRadiusChunks);

        const int loadedWorldX = static_cast<int>(std::floor(playerState.position.x));
        const int loadedWorldZ = static_cast<int>(std::floor(playerState.position.z));
        int loadedGroundY = m_VoxelWorld->FindHighestSolidBlockY(loadedWorldX, loadedWorldZ);
        if (loadedGroundY >= 0) {
            playerState.position.x = static_cast<float>(loadedWorldX) + 0.5f;
            playerState.position.y = static_cast<float>(loadedGroundY) + 1.0f + (Constants::PLAYER_HEIGHT * 0.5f) + 0.05f;
            playerState.position.z = static_cast<float>(loadedWorldZ) + 0.5f;
        }

        // Même logique de sécurité que pour le spawn initial: éviter de charger le joueur dans un bloc.
        playerState.position.y = findSafeSpawnY(playerState.position);

        m_Player->SetPosition(playerState.position);
        m_Camera.SetPosition(playerState.position + glm::vec3(0.0f, Constants::EYE_HEIGHT, 0.0f));
        m_Camera.SetRotation(playerState.yaw, playerState.pitch);
        std::cout << "[Game] État du joueur chargé depuis la sauvegarde" << std::endl;
    }

    // 4. Contrôleur
    m_EntityController = std::make_unique<NihilEngine::EntityController>(m_Camera, GLFW_KEY_TAB); //
    m_EntityController->AddControllableEntity(m_Player.get());

    // 5. UI
    m_DebugOverlay = std::make_unique<MonJeu::GameDebugOverlay>(1280, 720); //

    m_LastTime = static_cast<float>(glfwGetTime());
    std::cout << "[Game] Game Objects OK." << std::endl;
}


void Game::Run() {
    std::cout << "[Game] Starting main loop..." << std::endl;
    float fpsUpdateTimer = 0.0f;
    int frameCount = 0;

    while (!m_Window->ShouldClose()) {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - m_LastTime;
        m_LastTime = currentTime;

        // Calcul FPS
        frameCount++;
        fpsUpdateTimer += deltaTime;
        if (fpsUpdateTimer >= 1.0f) {
            m_FPS = static_cast<float>(frameCount) / fpsUpdateTimer;
            fpsUpdateTimer = 0.0f;
            frameCount = 0;
        }

        // Mises à jour
        ProcessInput(deltaTime);
        Update(deltaTime);
        Render();
    }
}

void Game::ProcessInput(float deltaTime) {
    if (NihilEngine::Input::IsKeyTriggered(GLFW_KEY_ESCAPE)) { //
        glfwSetWindowShouldClose(m_Window->GetGLFWWindow(), GLFW_TRUE);
    }
    if (NihilEngine::Input::IsKeyTriggered(GLFW_KEY_F3)) m_DebugOverlay->ToggleDebugInfo(); //
    if (NihilEngine::Input::IsKeyTriggered(GLFW_KEY_F4)) m_Player->ToggleRaycastVis(); //
    if (NihilEngine::Input::IsKeyTriggered(GLFW_KEY_F6)) m_DebugOverlay->TogglePerformance(); //
    if (NihilEngine::Input::IsKeyTriggered(GLFW_KEY_F7)) {
        const bool nextGpuMode = !m_VoxelWorld->IsTerrainGpuPreferred();
        m_VoxelWorld->SetTerrainGpuPreferred(nextGpuMode);
        std::cout << "[Game] Terrain generation backend switched to "
                  << (nextGpuMode ? "GPU preferred" : "CPU forced")
                  << " | GPU available: "
                  << (m_VoxelWorld->IsTerrainGpuAvailable() ? "yes" : "no")
                  << std::endl;

        if (nextGpuMode && !m_VoxelWorld->IsTerrainGpuAvailable()) {
            std::cout << "[Game] WARNING: GPU backend requested but unavailable. Terrain generation stays on CPU fallback." << std::endl;
        }
    }
    if (NihilEngine::Input::IsKeyTriggered(GLFW_KEY_F5)) {
        std::cout << "[Game] Sauvegarde manuelle du monde..." << std::endl;
        // La sauvegarde automatique se fait déjà dans UpdateDirtyChunks, mais on peut forcer
        // Pour l'instant, juste un message
        std::cout << "[Game] Monde sauvegardé (sauvegarde automatique active)" << std::endl;
    }

    // Interaction
    glm::vec3 origin = m_Camera.GetPosition();
    glm::vec3 direction = m_Camera.GetForward();

    if (NihilEngine::Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) { //
        NihilEngine::RaycastHit hit;
        if (m_VoxelWorld->Raycast(origin, direction, Constants::RAYCAST_DISTANCE, hit)) {
            m_VoxelWorld->SetVoxelActive(hit.blockPosition.x, hit.blockPosition.y, hit.blockPosition.z, false);
        }
    }
    if (NihilEngine::Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) { //
        NihilEngine::RaycastHit hit;
        if (m_VoxelWorld->Raycast(origin, direction, Constants::RAYCAST_DISTANCE, hit)) {
            glm::ivec3 placePos = hit.blockPosition + glm::ivec3(hit.hitNormal);
            m_VoxelWorld->SetVoxelActive(placePos.x, placePos.y, placePos.z, true);
        }
    }
}

void Game::SavePlayerState() {
    if (!m_WorldSaveManager) return;

    PlayerState state;
    state.position = m_Player->GetPosition();
    state.yaw = m_Camera.GetYaw();
    state.pitch = m_Camera.GetPitch();

    if (m_WorldSaveManager->SavePlayerState(state)) {
        // std::cout << "[Game] État du joueur sauvegardé" << std::endl;
    } else {
        std::cerr << "[Game] Erreur lors de la sauvegarde de l'état du joueur" << std::endl;
    }
}

void Game::Update(float deltaTime) {
    NihilEngine::PerformanceMonitor::getInstance().startFrame(); //

    // Moteur
    NihilEngine::PerformanceMonitor::getInstance().startSection("Physics");
    m_PhysicsWorld->update(deltaTime); //
    NihilEngine::PerformanceMonitor::getInstance().endSection("Physics");

    NihilEngine::PerformanceMonitor::getInstance().startSection("Environment");
    m_Environment->updateAtmosphere(deltaTime); //
    NihilEngine::PerformanceMonitor::getInstance().endSection("Environment");

    NihilEngine::PerformanceMonitor::getInstance().startSection("Audio");
    NihilEngine::AudioSystem::getInstance().setListenerPosition(m_Camera.GetPosition()); //
    NihilEngine::AudioSystem::getInstance().setListenerOrientation(m_Camera.GetForward(), m_Camera.GetUp()); //
    NihilEngine::AudioSystem::getInstance().update();
    NihilEngine::PerformanceMonitor::getInstance().endSection("Audio");

    // Jeu
    NihilEngine::PerformanceMonitor::getInstance().startSection("EntityController");
    m_EntityController->Update(deltaTime, *m_VoxelWorld); //
    NihilEngine::PerformanceMonitor::getInstance().endSection("EntityController");

    NihilEngine::PerformanceMonitor::getInstance().startSection("VoxelWorld_UpdateDirty");
    m_VoxelWorld->UpdateDirtyChunks(); //
    NihilEngine::PerformanceMonitor::getInstance().endSection("VoxelWorld_UpdateDirty");

    NihilEngine::PerformanceMonitor::getInstance().startSection("VoxelWorld_LOD");
    m_VoxelWorld->UpdateLOD(m_Camera.GetPosition(), deltaTime); //
    NihilEngine::PerformanceMonitor::getInstance().endSection("VoxelWorld_LOD");

    // Sauvegarde périodique de l'état du joueur (toutes les 5 secondes)
    m_PlayerSaveTimer += deltaTime;
    if (m_PlayerSaveTimer >= 5.0f) {
        SavePlayerState();
        m_PlayerSaveTimer = 0.0f;
    }

    NihilEngine::PerformanceMonitor::getInstance().endFrame(); //
    NihilEngine::Input::Update(); //
}

void Game::Render() {
    NihilEngine::PerformanceMonitor::getInstance().startSection("Render_Clear");
    m_Renderer->Clear(); //
    NihilEngine::PerformanceMonitor::getInstance().endSection("Render_Clear");

    // Config B brouillard (si activé)
    m_Renderer->EnableFog(false); //
    // ... (logique de brouillard) ...

    // Rendu 3D
    NihilEngine::PerformanceMonitor::getInstance().startSection("Render_VoxelWorld");
    m_VoxelWorld->Render(*m_Renderer, m_Camera); //
    NihilEngine::PerformanceMonitor::getInstance().endSection("Render_VoxelWorld");

    if (m_Player->IsRaycastVisible()) {
        NihilEngine::PerformanceMonitor::getInstance().startSection("Render_Raycast");
        m_Player->RenderRaycast(*m_Renderer, m_Camera, *m_VoxelWorld); //
        NihilEngine::PerformanceMonitor::getInstance().endSection("Render_Raycast");
    }

    // Rendu 2D (UI)
    glDisable(GL_DEPTH_TEST); //
    NihilEngine::PerformanceMonitor::getInstance().startSection("Render_UI");
    m_Renderer->DrawCrosshair(m_Window->GetWidth(), m_Window->GetHeight()); //

    if (m_DebugOverlay) {  // Vérification de sécurité
        std::stringstream terrainBackend;
        terrainBackend << std::fixed << std::setprecision(3)
                       << "Terrain backend: "
                       << (m_VoxelWorld->IsTerrainGpuPreferred() ? "GPU preferred" : "CPU forced")
                       << " | GPU available: "
                       << (m_VoxelWorld->IsTerrainGpuAvailable() ? "yes" : "no")
                       << " | Last gen: "
                       << (m_VoxelWorld->WasLastTerrainGenerationGpu() ? "GPU" : "CPU")
                       << " (" << m_VoxelWorld->GetLastTerrainGenerationMs() << " ms)";
        m_DebugOverlay->AddText(terrainBackend.str(), 10.0f, 190.0f);

        m_DebugOverlay->RenderDebugInfo( //
            m_FPS,
            static_cast<int>(m_VoxelWorld->GetChunkCount()),
            m_Camera.GetPosition(),
            m_Player->GetPosition(),
            NihilEngine::PerformanceMonitor::getInstance().getSections()
        );
    }
    NihilEngine::PerformanceMonitor::getInstance().endSection("Render_UI");
    glEnable(GL_DEPTH_TEST); //

    m_Window->OnUpdate(); // Swap buffers
}

} // namespace MonJeu