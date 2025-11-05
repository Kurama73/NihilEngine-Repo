# Bibliothèque NihilEngine
add_library(NihilEngine
    src/Audio.cpp
    src/BiomeGenerator.cpp
    src/Camera.cpp
    src/ChunkDataCache.cpp
    src/DebugOverlay.cpp
    src/Entity.cpp
    src/EntityController.cpp
    src/Environment.cpp
    src/Input.cpp
    src/LODMeshGenerator.cpp
    src/LODSystem.cpp
    src/Mesh.cpp
    src/Noise.cpp
    src/Performance.cpp
    src/Physics.cpp
    src/ProceduralGenerator.cpp
    src/ProgressiveChunkUpdate.cpp
    src/Renderer.cpp
    src/RiverGenerator.cpp
    src/TerrainGenerator.cpp
    src/TextRenderer.cpp
    src/TextureManager.cpp
    src/VegetationGenerator.cpp
    src/WaterGenerator.cpp
    src/Window.cpp
)

# Set C++ standard to 17
set_target_properties(NihilEngine PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)

# Cible les en-têtes pour que d'autres projets puissent les trouver
target_include_directories(NihilEngine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${BULLET_INCLUDE_DIRS}
    ${Stb_INCLUDE_DIR}
)

# Trouver les paquets vcpkg (GLFW, GLAD, GLM)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenAL CONFIG REQUIRED)
find_package(Bullet CONFIG REQUIRED)
find_package(Stb REQUIRED)

# Lier ces bibliothèques à notre moteur
target_link_libraries(NihilEngine PUBLIC
    glfw    # Fenêtre
    glad::glad     # OpenGL
    glm::glm       # Maths
    Freetype::Freetype  # FreeType pour le rendu de texte
    OpenAL::OpenAL  # Audio
    ${BULLET_LIBRARIES}  # Bullet Physics
)