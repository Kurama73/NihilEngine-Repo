#include <NihilEngine/Window.h>
#include <NihilEngine/Renderer.h>
#include <NihilEngine/Camera.h>
#include <NihilEngine/Input.h>
#include <NihilEngine/Mesh.h>
#include <NihilEngine/Entity.h>
#include <NihilEngine/Physics.h>
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
        NihilEngine::Window window(1280, 720, "Mon Jeu (Powered by NihilEngine)");
        NihilEngine::Renderer renderer;
        NihilEngine::Camera camera(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
        NihilEngine::Input::Init(window.GetGLFWWindow());

        // --- CORRECTION 1: Lier la caméra à la fenêtre ---
        // Cela permet à la fenêtre de notifier la caméra si elle est redimensionnée,
        // ce qui corrige le décalage de la visée.
        window.SetCamera(&camera);

        MonJeu::VoxelWorld voxelWorld;

        for (int chunkX = -3; chunkX <= 3; ++chunkX) {
            for (int chunkZ = -3; chunkZ <= 3; ++chunkZ) {
                voxelWorld.GenerateChunk(chunkX, chunkZ);
            }
        }

        std::cout << "All initialized" << std::endl;

        float lastTime = glfwGetTime();

        // Variables pour gérer le clic unique
        bool leftMouseWasPressed = false;
        bool rightMouseWasPressed = false;

        // Boucle principale
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
            glm::vec3 front = camera.GetForward();
            glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
            glm::vec3 up = glm::normalize(glm::cross(right, front));

            float speed = 5.0f * deltaTime;
            if (NihilEngine::Input::IsKeyPressed(GLFW_KEY_W)) position += front * speed;
            if (NihilEngine::Input::IsKeyPressed(GLFW_KEY_S)) position -= front * speed;
            if (NihilEngine::Input::IsKeyPressed(GLFW_KEY_A)) position -= right * speed;
            if (NihilEngine::Input::IsKeyPressed(GLFW_KEY_D)) position += right * speed;
            if (NihilEngine::Input::IsKeyPressed(GLFW_KEY_SPACE)) position.y += speed;
            if (NihilEngine::Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) position.y -= speed;
            camera.SetPosition(position);

            // --- Raycast unique (pour le laser ET les clics) ---
            glm::vec3 laserEndPoint;

            // --- CORRECTION 2: Offset du laser (Problème du 'near plane') ---
            // On passe de 0.1f à 0.2f pour être SÛR d'être DEVANT le near plane (0.1f)
            float laserOffset = 0.2f;
            glm::vec3 origin = camera.GetPosition() + camera.GetForward() * laserOffset;

            glm::vec3 direction = camera.GetForward();
            float maxDistance = 10.0f;

            NihilEngine::RaycastHit hitResult;
            bool didHit = voxelWorld.Raycast(origin, direction, maxDistance, hitResult);

            if (didHit) {
                // Le laser s'arrête au point d'impact
                laserEndPoint = hitResult.hitPoint;
            } else {
                // Le laser part à la distance maximale
                laserEndPoint = origin + direction * maxDistance;
            }

            // --- Gestion du clic unique ---
            bool leftMouseIsPressed = NihilEngine::Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
            bool rightMouseIsPressed = NihilEngine::Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT);

            bool leftMouseClicked = leftMouseIsPressed && !leftMouseWasPressed;
            bool rightMouseClicked = rightMouseIsPressed && !rightMouseWasPressed;

            // --- GESTION CLICS SOURIS (utilisant le raycast unique) ---
            if (leftMouseClicked) {
                if (didHit) { // On utilise le résultat du raycast déjà fait
                    voxelWorld.SetVoxelActive(hitResult.blockPosition.x, hitResult.blockPosition.y, hitResult.blockPosition.z, false);
                }
            }

            if (rightMouseClicked) {
                if (didHit) { // On utilise le résultat du raycast déjà fait
                    glm::ivec3 placePos = hitResult.blockPosition + glm::ivec3(hitResult.hitNormal);
                    voxelWorld.SetVoxelActive(placePos.x, placePos.y, placePos.z, true);
                }
            }

            // Mettre à jour l'état des boutons pour la prochaine image
            leftMouseWasPressed = NihilEngine::Input::IsMouseButtonHeld(GLFW_MOUSE_BUTTON_LEFT);
            rightMouseWasPressed = NihilEngine::Input::IsMouseButtonHeld(GLFW_MOUSE_BUTTON_RIGHT);


            // --- MISE À JOUR ---
            voxelWorld.UpdateDirtyChunks();

            // --- RENDU ---
            renderer.Clear();
            voxelWorld.Render(renderer, camera);

            // --- Dessiner le laser ---
            // On utilise aussi l'offset de 0.2f ici
            glm::vec3 laserStart = camera.GetPosition() + camera.GetForward() * laserOffset;
            renderer.DrawLine3D(laserStart, laserEndPoint, camera, glm::vec3(1.0f, 0.0f, 0.0f)); // Rouge

            // --- Dessiner le viseur ---
            // Il utilise la taille actuelle de la fenêtre pour rester centré
            renderer.DrawCrosshair(window.GetWidth(), window.GetHeight());

            // --- SYNCHRONISATION ---
            NihilEngine::Input::Update();
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