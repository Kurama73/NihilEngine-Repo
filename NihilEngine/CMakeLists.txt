# Nom du projet Moteur
project(NihilEngine)

# Créer une bibliothèque (STATIC = .lib)
add_library(NihilEngine STATIC
    src/Window.cpp
    src/Renderer.cpp
    src/Camera.cpp
    src/Input.cpp
    src/Mesh.cpp
    src/Entity.cpp
    include/NihilEngine/Window.h
    include/NihilEngine/Renderer.h
    include/NihilEngine/Camera.h
    include/NihilEngine/Input.h
    include/NihilEngine/Mesh.h
    include/NihilEngine/Entity.h
)

# Rendre le dossier "include" public
# (PUBLIC = tous ceux qui linkent NihilEngine reçoivent ce chemin d'include)
target_include_directories(NihilEngine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
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