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

        MonJeu::VoxelWorld voxelWorld;

        for (int chunkX = -3; chunkX <= 3; ++chunkX) {
            for (int chunkZ = -3; chunkZ <= 3; ++chunkZ) {
                voxelWorld.GenerateChunk(chunkX, chunkZ);
            }
        }

        std::cout << "All initialized" << std::endl;

        float lastTime = glfwGetTime();

        // --- NOUVEAU: Variable pour le laser ---
        glm::vec3 laserEndPoint = camera.GetPosition(); // Initialiser

        // 2. Boucle principale
        while (!window.ShouldClose())
        {
            float currentTime = glfwGetTime();
            float deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            // --- INPUT CAMÉRA ---
            double mouseDx, mouseDy;
            NihilEngine::Input::GetMouseDelta(mouseDx, mouseDy);
            // La rotation de la caméra va maintenant fonctionner
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


            // --- NOUVEAU: Raycast continu pour le laser ---
            // On le fait à chaque image, que l'on clique ou non
            {
                NihilEngine::RaycastHit laserHit;
                glm::vec3 origin = camera.GetPosition();
                glm::vec3 direction = camera.GetForward(); // Le vecteur est maintenant correct

                if (voxelWorld.Raycast(origin, direction, 50.0f, laserHit)) {
                    // Si on touche, le laser s'arrête au centre du bloc
                    laserEndPoint = glm::vec3(laserHit.blockPosition) + 0.5f;
                } else {
                    // Si on ne touche rien, le laser part au loin
                    laserEndPoint = origin + direction * 50.0f;
                }
            }


            // --- GESTION CLICS SOURIS ---
            // (Ce code est maintenant correct grâce à la réparation de Input.cpp)
            if (NihilEngine::Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
                NihilEngine::RaycastHit result;
                glm::vec3 origin = camera.GetPosition();
                glm::vec3 direction = camera.GetForward();

                if (voxelWorld.Raycast(origin, direction, 10.0f, result)) {
                    voxelWorld.SetVoxelActive(result.blockPosition.x, result.blockPosition.y, result.blockPosition.z, false);
                }
            }

            if (NihilEngine::Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
                NihilEngine::RaycastHit result;
                glm::vec3 origin = camera.GetPosition();
                glm::vec3 direction = camera.GetForward();

                if (voxelWorld.Raycast(origin, direction, 10.0f, result)) {
                    glm::ivec3 placePos = result.blockPosition + glm::ivec3(result.hitNormal);
                    voxelWorld.SetVoxelActive(placePos.x, placePos.y, placePos.z, true);
                }
            }

            // --- MISE À JOUR ---
            voxelWorld.UpdateDirtyChunks();

            // --- RENDU ---
            renderer.Clear();
            voxelWorld.Render(renderer, camera);

            // --- NOUVEAU: Dessiner le laser ---
            // On dessine une ligne rouge du "nez" de la caméra jusqu'au point d'impact
            glm::vec3 laserStart = camera.GetPosition() + camera.GetForward() * 0.1f; // Partir juste devant
            renderer.DrawLine3D(laserStart, laserEndPoint, camera, glm::vec3(1.0f, 0.0f, 0.0f));

            renderer.DrawCrosshair(1280, 720);

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