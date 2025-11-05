// src/Game.cpp
#include <MonJeu/Game.h>
#include <NihilEngine/Input.h>
#include <NihilEngine/Audio.h>
#include <NihilEngine/TextureManager.h>
#include <NihilEngine/Performance.h>
#include <MonJeu/Constants.h>
#include <GLFW/glfw3.h>
#include <iostream>
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

    // 3. Joueur
    m_Player = std::make_unique<MonJeu::Player>(); //

    // Spawn du joueur
    NihilEngine::TerrainGenerator& terrainGen = m_VoxelWorld->GetProceduralGenerator().getTerrainGenerator(); //
    float spawnHeight = terrainGen.getHeight(0.5f, 0.5f); //
    glm::vec3 spawnPos = glm::vec3(0.5f, spawnHeight + Constants::PLAYER_HEIGHT, 0.5f); //
    m_Player->SetPosition(spawnPos);
    m_Camera.SetPosition(spawnPos + glm::vec3(0.0f, Constants::EYE_HEIGHT, 0.0f)); //

    // Orienter la caméra vers -Z (devant le joueur) au lieu de +Z
    m_Camera.SetRotation(270.0f, 0.0f); // 270 degrés = -Z direction

    // Charger l'état du joueur s'il existe
    MonJeu::PlayerState playerState;
    if (m_WorldSaveManager->LoadPlayerState(playerState)) {
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
    m_PhysicsWorld->update(deltaTime); //
    m_Environment->updateAtmosphere(deltaTime); //
    NihilEngine::AudioSystem::getInstance().setListenerPosition(m_Camera.GetPosition()); //
    NihilEngine::AudioSystem::getInstance().setListenerOrientation(m_Camera.GetForward(), m_Camera.GetUp()); //
    NihilEngine::AudioSystem::getInstance().update();

    // Jeu
    m_EntityController->Update(deltaTime, *m_VoxelWorld); //
    m_VoxelWorld->UpdateDirtyChunks(); //
    m_VoxelWorld->UpdateLOD(m_Camera.GetPosition(), deltaTime); //

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
    m_Renderer->Clear(); //

    // Config B brouillard (si activé)
    m_Renderer->EnableFog(false); //
    // ... (logique de brouillard) ...

    // Rendu 3D
    m_VoxelWorld->Render(*m_Renderer, m_Camera); //
    m_EntityController->Render(*m_Renderer, m_Camera, true); //

    if (m_Player->IsRaycastVisible()) {
        m_Player->RenderRaycast(*m_Renderer, m_Camera, *m_VoxelWorld); //
    }

    // Rendu 2D (UI)
    glDisable(GL_DEPTH_TEST); //
    m_Renderer->DrawCrosshair(m_Window->GetWidth(), m_Window->GetHeight()); //

    if (m_DebugOverlay) {  // Vérification de sécurité
        m_DebugOverlay->RenderDebugInfo( //
            m_FPS,
            static_cast<int>(m_VoxelWorld->GetChunkCount()),
            m_Camera.GetPosition(),
            m_Player->GetPosition()
        );
    }
    glEnable(GL_DEPTH_TEST); //

    m_Window->OnUpdate(); // Swap buffers
}

} // namespace MonJeu