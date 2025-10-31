# Nom du projet Jeu
project(MonJeu)

# Créer un exécutable (.exe)
add_executable(MonJeu
    src/main.cpp
)

# Lier notre jeu à notre moteur
# (PRIVATE = on utilise NihilEngine)
# CMake va automatiquement gérer tous les liens (GLFW, GLAD...)
target_link_libraries(MonJeu PRIVATE
    NihilEngine
)