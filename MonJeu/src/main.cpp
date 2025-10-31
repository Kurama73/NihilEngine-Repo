// Inclure la classe Fenêtre de NOTRE moteur
#include <NihilEngine/Window.h>
#include <NihilEngine/Renderer.h>
#include <NihilEngine/Camera.h>
#include <NihilEngine/Input.h>
#include <NihilEngine/Mesh.h>
#include <NihilEngine/Entity.h>

// Inclure notre système voxel
#include <MonJeu/VoxelWorld.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <memory>

int main()
{
    try {
        std::cout << "Starting MonJeu..." << std::endl;
    // 1. Le MOTEUR crée la fenêtre
    NihilEngine::Window window(1280, 720, "Mon Jeu (Powered by NihilEngine)");
    NihilEngine::Renderer renderer;
    NihilEngine::Camera camera(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
    NihilEngine::Input::Init(window.GetGLFWWindow());

    // Créer le monde voxel
    MonJeu::VoxelWorld voxelWorld;

    // Générer plusieurs chunks autour de (0,0)
    for (int chunkX = -1; chunkX <= 1; ++chunkX) {
        for (int chunkZ = -1; chunkZ <= 1; ++chunkZ) {
            voxelWorld.GenerateChunk(chunkX, chunkZ);
        }
    }

    // Créer les entités pour les chunks
    std::vector<std::unique_ptr<NihilEngine::Entity>> entities;

    for (const auto& pair : voxelWorld.GetChunks()) {
        const MonJeu::Chunk& chunk = pair.second;

        // CreateMesh() retourne un temporaire → on le déplace directement
        auto chunkEntity = std::make_unique<NihilEngine::Entity>(
            std::move(chunk.CreateMesh()),  // std::move obligatoire
            glm::vec3(
                chunk.GetChunkX() * MonJeu::Chunk::SIZE - (MonJeu::Chunk::SIZE - 1) / 2.0f,
                0.0f,
                chunk.GetChunkZ() * MonJeu::Chunk::SIZE - (MonJeu::Chunk::SIZE - 1) / 2.0f
            )
        );
        chunkEntity->SetColor(glm::vec4(0.2f, 0.8f, 0.2f, 1.0f)); // Vert
        entities.push_back(std::move(chunkEntity));
    }

    std::cout << "All initialized - " << entities.size() << " chunk(s) loaded" << std::endl;

    float lastTime = glfwGetTime();

    // 2. Boucle principale
    while (!window.ShouldClose())
    {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // --- INPUT CAMÉRA ---
        double mouseDx, mouseDy;
        NihilEngine::Input::GetMouseDelta(mouseDx, mouseDy);
        camera.SetRotation(camera.GetYaw() + mouseDx * 0.1f, camera.GetPitch() + mouseDy * 0.1f);

        glm::vec3 position = camera.GetPosition();
        float yawRad = glm::radians(camera.GetYaw());
        glm::vec3 front = glm::normalize(glm::vec3(cos(yawRad), 0.0f, sin(yawRad)));
        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));

        float speed = 5.0f * deltaTime;
        if (NihilEngine::Input::IsKeyPressed(GLFW_KEY_W)) position += front * speed;
        if (NihilEngine::Input::IsKeyPressed(GLFW_KEY_S)) position -= front * speed;
        if (NihilEngine::Input::IsKeyPressed(GLFW_KEY_A)) position -= right * speed;
        if (NihilEngine::Input::IsKeyPressed(GLFW_KEY_D)) position += right * speed;
        if (NihilEngine::Input::IsKeyPressed(GLFW_KEY_SPACE)) position.y += speed;
        if (NihilEngine::Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) position.y -= speed;
        camera.SetPosition(position);

        // --- GESTION CLICS SOURIS ---
        if (NihilEngine::Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
            std::cout << "Clic left" << std::endl;
            MonJeu::VoxelWorld::RaycastResult result;
            glm::vec3 origin = camera.GetPosition();
            glm::vec3 direction = camera.GetForward();
            if (voxelWorld.Raycast(origin, direction, 10.0f, result)) {
                // Casser le bloc
                voxelWorld.SetVoxelActive(result.x + result.chunkX * MonJeu::Chunk::SIZE, result.y, result.z + result.chunkZ * MonJeu::Chunk::SIZE, false);
                // Recréer les entities
                entities.clear();
                for (const auto& pair : voxelWorld.GetChunks()) {
                    const MonJeu::Chunk& chunk = pair.second;
                    int chunkX = (pair.first >> 32);
                    int chunkZ = (pair.first & 0xFFFFFFFF);
                    auto chunkEntity = std::make_unique<NihilEngine::Entity>(
                        std::move(chunk.CreateMesh()),
                        glm::vec3(chunkX * MonJeu::Chunk::SIZE - (MonJeu::Chunk::SIZE - 1) / 2.0f, 0.0f, chunkZ * MonJeu::Chunk::SIZE - (MonJeu::Chunk::SIZE - 1) / 2.0f)
                    );
                    entities.push_back(std::move(chunkEntity));
                }
            }
        }

        if (NihilEngine::Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
            std::cout << "Clic right" << std::endl;
            MonJeu::VoxelWorld::RaycastResult result;
            glm::vec3 origin = camera.GetPosition();
            glm::vec3 direction = camera.GetForward();
            if (voxelWorld.Raycast(origin, direction, 10.0f, result)) {
                // Placer un bloc adjacent
                glm::vec3 placePos = glm::vec3(result.x + result.chunkX * MonJeu::Chunk::SIZE, result.y, result.z + result.chunkZ * MonJeu::Chunk::SIZE) + result.normal;
                voxelWorld.SetVoxelActive((int)placePos.x, (int)placePos.y, (int)placePos.z, true);
                // Recréer les entities
                entities.clear();
                for (const auto& pair : voxelWorld.GetChunks()) {
                    const MonJeu::Chunk& chunk = pair.second;
                    int chunkX = (pair.first >> 32);
                    int chunkZ = (pair.first & 0xFFFFFFFF);
                    auto chunkEntity = std::make_unique<NihilEngine::Entity>(
                        std::move(chunk.CreateMesh()),
                        glm::vec3(chunkX * MonJeu::Chunk::SIZE - (MonJeu::Chunk::SIZE - 1) / 2.0f, 0.0f, chunkZ * MonJeu::Chunk::SIZE - (MonJeu::Chunk::SIZE - 1) / 2.0f)
                    );
                    entities.push_back(std::move(chunkEntity));
                }
            }
        }

        // --- RENDU ---
        renderer.Clear();

        // Blocs normaux d'abord
        for (const auto& entity : entities) {
            renderer.DrawMesh(
                entity->GetMesh(),
                camera,
                entity->GetModelMatrix()
            );
        }

        // Outline en wireframe noir
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(2.0f);
        glDisable(GL_DEPTH_TEST); // Pour que les lignes soient toujours visibles
        for (const auto& entity : entities) {
            renderer.DrawMesh(
                entity->GetMesh(),
                camera,
                entity->GetModelMatrix(),
                glm::vec3(0.0f)
            );
        }
        glEnable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        window.OnUpdate();
    }

    return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        return 1;
    }
}