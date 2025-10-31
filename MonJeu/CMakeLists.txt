# Nom du projet Jeu
project(MonJeu)

# Créer un exécutable (.exe)
add_executable(MonJeu
    src/main.cpp
    src/VoxelWorld.cpp
)

# Inclure le dossier include de MonJeu
target_include_directories(MonJeu PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Lier notre jeu à notre moteur
# (PRIVATE = on utilise NihilEngine)
# CMake va automatiquement gérer tous les liens (GLFW, GLAD...)
target_link_libraries(MonJeu PRIVATE
    NihilEngine
)