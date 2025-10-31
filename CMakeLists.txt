# Version minimale de CMake
cmake_minimum_required(VERSION 3.15)

# Nom du projet global
project(NihilProject VERSION 1.0)

# Définir le standard C++ (C++17 est un bon choix)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dire à CMake d'aller voir dans les sous-dossiers
add_subdirectory(NihilEngine)
add_subdirectory(MonJeu)