# Exemple d'utilisation des nouvelles capacités de NihilEngine

## Initialisation des systèmes

```cpp
#include <NihilEngine/Audio.h>
#include <NihilEngine/Physics.h>
#include <NihilEngine/Renderer.h>
#include <NihilEngine/TextureManager.h>
#include <NihilEngine/Environment.h>
#include <NihilEngine/Performance.h>

// Dans votre fonction d'initialisation
void initializeEngine() {
    // Audio
    AudioSystem::getInstance().initialize();

    // Physique
    PhysicsWorld* physicsWorld = new PhysicsWorld();
    physicsWorld->initialize();

    // Textures Minecraft
    TextureManager::getInstance().loadMinecraftTexturePack("path/to/minecraft/resourcepack");

    // Environnement
    Environment* environment = new Environment();
    environment->setTimeOfDay(0.5f); // Midi

    // Performance
    PerformanceMonitor::getInstance();
    LODManager lodManager;
    lodManager.enableVSync(true);
}
```

## Utilisation du système audio

```cpp
// Charger et jouer un son
AudioBuffer* buffer = AudioSystem::getInstance().createBuffer();
buffer->loadFromFile("sounds/block_break.wav");

AudioSource* source = AudioSystem::getInstance().createSource();
source->setBuffer(buffer);
source->setPosition(playerPosition);
source->play();

// Mise à jour audio (dans la boucle principale)
AudioSystem::getInstance().setListenerPosition(cameraPosition);
AudioSystem::getInstance().setListenerOrientation(cameraForward, cameraUp);
AudioSystem::getInstance().update();
```

## Utilisation de la physique avancée

```cpp
// Créer un corps rigide
RigidBody* box = physicsWorld->createRigidBody(1.0f, glm::vec3(0, 10, 0), new btBoxShape(btVector3(1, 1, 1)));

// Appliquer des forces
box->applyForce(glm::vec3(0, 0, 10));

// Mise à jour physique (dans la boucle principale)
physicsWorld->update(deltaTime);

// Obtenir la position mise à jour
glm::vec3 position = box->getPosition();
```

## Utilisation du rendu immersif

```cpp
// Activer le brouillard
renderer.EnableFog(true);
renderer.SetFogColor(glm::vec3(0.5f, 0.5f, 0.5f));
renderer.SetFogDensity(0.01f);

// Ajouter des particules
Particle rainParticle;
rainParticle.position = glm::vec3(0, 10, 0);
rainParticle.velocity = glm::vec3(0, -5, 0);
rainParticle.color = glm::vec4(0.7f, 0.7f, 1.0f, 0.8f);
rainParticle.life = 5.0f;
rainParticle.size = 0.1f;
renderer.AddParticle(rainParticle);

// Mise à jour des particules
renderer.UpdateParticles(deltaTime);
renderer.DrawParticles(camera);
```

## Utilisation des textures Minecraft

```cpp
// Charger une texture de bloc
GLuint grassTexture = TextureManager::getInstance().getTexture("block/grass_block_top");

// Dans le rendu d'une entité
entity.SetTexture(grassTexture);
renderer.DrawEntity(entity, camera);
```

## Utilisation de l'environnement et de l'immersion

```cpp
// Configurer le temps et la météo
environment->setTimeOfDay(0.75f); // Soirée
environment->setWeather("rain", 0.8f);

// Ajouter des effets atmosphériques
AtmosphericEffect fog;
fog.type = "fog";
fog.intensity = 0.5f;
fog.color = glm::vec3(0.6f, 0.6f, 0.7f);
fog.density = 0.02f;
environment->addAtmosphericEffect(fog);

// Mise à jour
environment->updateAtmosphere(deltaTime);
environment->updateWeather(deltaTime);

// Utiliser dans le rendu
glm::vec3 skyColor = environment->getSkyColor();
glm::vec3 ambientLight = environment->getAmbientLight();
```

## Optimisations de performance

```cpp
// Monitorer les performances
PerformanceMonitor::getInstance().startFrame();
// ... votre code de rendu ...
PerformanceMonitor::getInstance().endFrame();

float fps = PerformanceMonitor::getInstance().getFPS();
float frameTime = PerformanceMonitor::getInstance().getFrameTime();

// Utiliser LOD pour les objets distants
float detail = lodManager.getDetailLevel(objectPosition, cameraPosition);
if (detail > 0.5f) {
    // Rendu détaillé
    renderHighDetail(object);
} else {
    // Rendu simplifié
    renderLowDetail(object);
}
```

## Architecture recommandée pour les jeux

```cpp
class Game : public IControllableEntity {
public:
    void initialize() {
        // Initialiser tous les systèmes
        initializeEngine();

        // Créer le monde voxel
        voxelWorld = new VoxelWorld();

        // Configurer l'audio spatial
        setupAudio();

        // Configurer la physique
        setupPhysics();
    }

    void update(float deltaTime) {
        // Mise à jour des systèmes
        physicsWorld->update(deltaTime);
        AudioSystem::getInstance().update();
        environment->updateAtmosphere(deltaTime);
        renderer.UpdateParticles(deltaTime);

        // Logique de jeu
        updateGameLogic(deltaTime);

        // Performance
        PerformanceMonitor::getInstance().startFrame();
        render();
        PerformanceMonitor::getInstance().endFrame();
    }

    void render() {
        renderer.Clear();

        // Rendu du monde avec LOD
        voxelWorld->renderWithLOD(camera, lodManager);

        // Rendu des entités
        for (auto& entity : entities) {
            renderer.DrawEntity(*entity, camera);
        }

        // Rendu des particules et effets
        renderer.DrawParticles(camera);
    }

private:
    PhysicsWorld* physicsWorld;
    Environment* environment;
    VoxelWorld* voxelWorld;
    LODManager lodManager;
    std::vector<Entity*> entities;
};
```

Cette architecture vous permet de développer rapidement des jeux immersifs avec des fonctionnalités avancées tout en maintenant de bonnes performances grâce aux systèmes d'optimisation intégrés.