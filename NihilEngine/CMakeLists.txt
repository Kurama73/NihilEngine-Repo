# Bibliothèque NihilEngine
add_library(NihilEngine
    src/Camera.cpp
    src/DebugOverlay.cpp
    src/Entity.cpp
    src/Input.cpp
    src/Mesh.cpp
    src/Physics.cpp
    src/Renderer.cpp
    src/TextRenderer.cpp
    src/Window.cpp
)

# Set C++ standard to 17
set_target_properties(NihilEngine PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)

# Cible les en-têtes pour que d'autres projets puissent les trouver
target_include_directories(NihilEngine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Trouver les paquets vcpkg (GLFW, GLAD, GLM)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Freetype REQUIRED)

# Lier ces bibliothèques à notre moteur
target_link_libraries(NihilEngine PUBLIC
    glfw    # Fenêtre
    glad::glad     # OpenGL
    glm::glm       # Maths
    Freetype::Freetype  # FreeType pour le rendu de texte
)