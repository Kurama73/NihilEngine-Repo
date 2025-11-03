# 1. Créer une BIBLIOTHÈQUE pour la logique du jeu
# Cela permet aux tests de se lier à la logique du jeu sans l'exécutable
add_library(MonJeuLib
    src/VoxelWorld.cpp
)

# La bibliothèque a besoin des en-têtes du moteur et de ses propres en-têtes
target_include_directories(MonJeuLib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${CMAKE_SOURCE_DIR}/NihilEngine/include # Accès au moteur
)

# Lie la logique du jeu au moteur
target_link_libraries(MonJeuLib PUBLIC NihilEngine)

# 2. Créer l'exécutable du jeu
add_executable(MonJeu
    src/main.cpp
)

# Lier l'exécutable à la bibliothèque de logique du jeu
target_link_libraries(MonJeu PUBLIC MonJeuLib)