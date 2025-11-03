# Bibliothèque NihilEngine
add_library(NihilEngine
    src/Camera.cpp
    src/Entity.cpp
    src/Input.cpp
    src/Mesh.cpp
    src/Renderer.cpp
    src/Window.cpp
)

# Cible les en-têtes pour que d'autres projets puissent les trouver
target_include_directories(NihilEngine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Trouver les paquets vcpkg (GLFW, GLAD, GLM)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

# Lier ces bibliothèques à notre moteur
target_link_libraries(NihilEngine PUBLIC
    glfw    # Fenêtre
    glad::glad     # OpenGL
    glm::glm       # Maths
)