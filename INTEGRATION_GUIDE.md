# Guide d'intégration des systèmes avancés dans MonJeu

## 📋 **Plan d'Intégration - Étape par Étape**

### **Étape 1 : Initialisation des Systèmes de Base**

Modifiez `main.cpp` pour initialiser tous les nouveaux systèmes :

```cpp
#include <NihilEngine/Audio.h>
#include <NihilEngine/Physics.h>
#include <NihilEngine/TextureManager.h>
#include <NihilEngine/Environment.h>
#include <NihilEngine/Performance.h>

// Dans la section INITIALISATION de main.cpp
// Après NihilEngine::Renderer renderer;

// === INITIALISATION DES NOUVEAUX SYSTÈMES ===

// Audio System
if (!NihilEngine::AudioSystem::getInstance().initialize()) {
    std::cerr << "Failed to initialize audio system!" << std::endl;
    return -1;
}

// Physics World
NihilEngine::PhysicsWorld* physicsWorld = new NihilEngine::PhysicsWorld();
physicsWorld->initialize();

// Texture Manager (avec support Minecraft)
NihilEngine::TextureManager::getInstance().loadMinecraftTexturePack("path/to/minecraft/resourcepack");

// Environment System
NihilEngine::Environment* environment = new NihilEngine::Environment();
environment->setTimeOfDay(0.5f); // Midi
environment->setWeather("clear", 1.0f);

// Performance Monitor
NihilEngine::PerformanceMonitor::getInstance();
NihilEngine::LODManager lodManager;
```

### **Étape 2 : Intégration Audio dans Player**

Modifiez `Player.h` pour ajouter le système audio :

```cpp
#include <NihilEngine/Audio.h>

class Player {
    // ... membres existants ...

private:
    NihilEngine::AudioSource* m_FootstepSource;
    NihilEngine::AudioBuffer* m_FootstepBuffer;
    NihilEngine::AudioSource* m_JumpSource;
    NihilEngine::AudioBuffer* m_JumpBuffer;

    // ... autres membres ...
};
```

Puis dans `Player.cpp`, initialisez et utilisez l'audio :

```cpp
Player::Player() {
    // ... initialisation existante ...

    // Audio initialization
    m_FootstepBuffer = NihilEngine::AudioSystem::getInstance().createBuffer();
    m_FootstepBuffer->loadFromFile("assets/sounds/footstep.wav");

    m_JumpBuffer = NihilEngine::AudioSystem::getInstance().createBuffer();
    m_JumpBuffer->loadFromFile("assets/sounds/jump.wav");

    m_FootstepSource = NihilEngine::AudioSystem::getInstance().createSource();
    m_FootstepSource->setBuffer(m_FootstepBuffer);
    m_FootstepSource->setVolume(0.7f);

    m_JumpSource = NihilEngine::AudioSystem::getInstance().createSource();
    m_JumpSource->setBuffer(m_JumpBuffer);
    m_JumpSource->setVolume(0.8f);
}

void Player::Update(float deltaTime, NihilEngine::Camera& camera, VoxelWorld& world, bool isCurrent) {
    // ... logique existante ...

    // Update audio position
    if (isCurrent) {
        NihilEngine::AudioSystem::getInstance().setListenerPosition(m_Position);
        NihilEngine::AudioSystem::getInstance().setListenerOrientation(
            camera.GetForward(), camera.GetUp());
    }

    m_FootstepSource->setPosition(m_Position);
    m_JumpSource->setPosition(m_Position);

    // Play footstep sound when moving
    if (glm::length(m_Velocity) > 0.1f && !m_IsFlying) {
        static float footstepTimer = 0.0f;
        footstepTimer += deltaTime;
        if (footstepTimer > 0.4f) { // Every 400ms
            m_FootstepSource->play();
            footstepTimer = 0.0f;
        }
    }

    // ... reste de la logique ...
}
```

### **Étape 3 : Physique Avancée dans VoxelWorld**

Modifiez `VoxelWorld.h` pour intégrer Bullet Physics :

```cpp
#include <NihilEngine/Physics.h>

class VoxelWorld {
    // ... membres existants ...

private:
    NihilEngine::PhysicsWorld* m_PhysicsWorld;
    std::vector<NihilEngine::RigidBody*> m_PhysicsBodies;

    // ... autres membres ...
};
```

Puis dans `VoxelWorld.cpp` :

```cpp
VoxelWorld::VoxelWorld() : m_PhysicsWorld(nullptr) {
    // ... initialisation existante ...
}

void VoxelWorld::Initialize(NihilEngine::PhysicsWorld* physicsWorld) {
    m_PhysicsWorld = physicsWorld;

    // Create physics bodies for blocks that need physics
    // (ex: falling sand, interactive objects)
}

void VoxelWorld::Update(float deltaTime) {
    // ... logique existante ...

    // Update physics bodies
    for (auto* body : m_PhysicsBodies) {
        // Synchronize physics position with visual position
        glm::vec3 physicsPos = body->getPosition();
        // Update corresponding visual block/entity
    }
}

bool VoxelWorld::RaycastBlocks(const glm::vec3& origin, const glm::vec3& direction,
                              float maxDistance, NihilEngine::RaycastHit& hit) {
    // Use the existing voxel raycast
    return NihilEngine::RaycastVoxel(origin, direction, maxDistance,
        [this](const glm::ivec3& pos) {
            return IsBlockAt(pos); // Your existing block check
        }, hit);
}
```

### **Étape 4 : Environnement Immersif**

Dans la boucle principale de `main.cpp`, ajoutez :

```cpp
// Dans la boucle while (!window.ShouldClose())
{
    // ... logique existante ...

    // Update environment
    environment->updateAtmosphere(deltaTime);
    environment->updateWeather(deltaTime);
    environment->updateLights(deltaTime);

    // Apply environment to renderer
    renderer.EnableFog(environment->getEffects().size() > 0);
    if (renderer.IsFogEnabled()) {
        // Set fog based on weather
        auto effects = environment->getEffects();
        for (const auto& effect : effects) {
            if (effect.type == "fog") {
                renderer.SetFogColor(effect.color);
                renderer.SetFogDensity(effect.density);
            }
        }
    }

    // Update sky color based on time of day
    glm::vec3 skyColor = environment->getSkyColor();
    // Use skyColor for skybox or clear color

    // ... rendu existant ...
}
```

### **Étape 5 : Textures Minecraft**

Modifiez le système de rendu pour utiliser les textures Minecraft :

```cpp
// Dans VoxelWorld::Render()
void VoxelWorld::Render(NihilEngine::Renderer& renderer, const NihilEngine::Camera& camera,
                       NihilEngine::LODManager& lodManager) {

    // Pour chaque bloc visible
    for (const auto& block : visibleBlocks) {
        float distance = glm::distance(block.position, camera.GetPosition());
        float detailLevel = lodManager.getDetailLevel(block.position, camera.GetPosition());

        if (detailLevel > 0.1f) { // Only render if detail level is sufficient
            // Get Minecraft texture
            std::string textureName = GetBlockTextureName(block.type);
            GLuint texture = NihilEngine::TextureManager::getInstance().getTexture(textureName);

            if (texture != 0) {
                // Create entity with Minecraft texture
                NihilEngine::Entity blockEntity;
                blockEntity.SetPosition(block.position);
                blockEntity.GetMaterial().textureID = texture;

                // Apply LOD scaling
                glm::vec3 scale = glm::vec3(detailLevel);
                blockEntity.SetScale(scale);

                renderer.DrawEntity(blockEntity, camera);
            }
        }
    }
}
```

### **Étape 6 : Monitoring Performance**

Ajoutez dans la boucle principale :

```cpp
// Au début de la boucle
NihilEngine::PerformanceMonitor::getInstance().startFrame();

// ... toute la logique de jeu ...

// À la fin de la boucle, avant glfwSwapBuffers
renderer.Clear(); // Votre rendu existant
// ... rendu ...
window.SwapBuffers();

NihilEngine::PerformanceMonitor::getInstance().endFrame();

// Afficher les stats de performance
float fps = NihilEngine::PerformanceMonitor::getInstance().getFPS();
debugOverlay.AddText("FPS: " + std::to_string((int)fps), 10, 10);
```

### **Étape 7 : Particules et Effets**

Ajoutez des effets visuels :

```cpp
// Dans Player.cpp, quand le joueur saute
void Player::Jump() {
    // ... logique de saut existante ...

    // Add jump particles
    for (int i = 0; i < 10; ++i) {
        NihilEngine::Particle particle;
        particle.position = m_Position - glm::vec3(0, m_Height/2, 0);
        particle.velocity = glm::vec3(
            (rand() % 200 - 100) / 100.0f,  // -1 to 1
            rand() % 200 / 100.0f,          // 0 to 2
            (rand() % 200 - 100) / 100.0f   // -1 to 1
        );
        particle.color = glm::vec4(0.8f, 0.6f, 0.2f, 1.0f); // Dust color
        particle.life = 1.0f + (rand() % 100) / 100.0f; // 1-2 seconds
        particle.size = 0.05f;

        // Vous devrez passer renderer à Player ou gérer ça ailleurs
        // renderer.AddParticle(particle);
    }

    // Play jump sound
    m_JumpSource->play();
}
```

## 🎯 **Structure Finale Recommandée**

```
MonJeu/
├── src/
│   ├── main.cpp          # Boucle principale + initialisation systèmes
│   ├── Player.cpp        # Joueur avec audio + physique
│   ├── VoxelWorld.cpp    # Monde voxel avec physique + textures
│   └── GameDebugOverlay.cpp
├── include/MonJeu/
│   ├── Player.h
│   ├── VoxelWorld.h
│   └── GameDebugOverlay.h
└── assets/
    ├── sounds/           # Fichiers audio
    ├── textures/         # Textures supplémentaires
    └── minecraft/        # Resource pack Minecraft
```

## 🚀 **Prochaines Étapes**

1. **Commencez par l'audio** - C'est le plus simple à intégrer
2. **Ajoutez la physique** - Pour les interactions réalistes
3. **Textures Minecraft** - Pour l'immersion visuelle
4. **Environnement** - Pour l'atmosphère
5. **Optimisations** - Pour les performances

Chaque système peut être ajouté indépendamment. Commencez petit et itérez ! 🎮