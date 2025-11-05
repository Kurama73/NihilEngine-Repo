cmake_minimum_required(VERSION 3.16)
project(NihilProject)

# Ajoute le moteur
add_subdirectory(NihilEngine)

# Ajoute le jeu
add_subdirectory(MonJeu)

# Exécutable de test pour le système procédural
add_executable(TestProcedural test_procedural.cpp)
target_link_libraries(TestProcedural PRIVATE NihilEngine)