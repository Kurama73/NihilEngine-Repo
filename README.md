# NihilEngine - Moteur de Jeu 3D From Scratch

## Description

NihilEngine est un moteur de jeu 3D développé entièrement from scratch en C++17, utilisant OpenGL pour le rendu. Ce projet vise à créer un environnement modulaire et réutilisable pour des jeux 3D, avec MonJeu comme exemple d'application pour un clone de Minecraft (génération de monde voxel, blocs, etc.).

Le moteur est conçu pour être simple, extensible et performant, en se concentrant sur les fondamentaux : fenêtrage, rendu, caméra, entrées, et gestion de scènes.

## Prérequis

Avant de commencer, assurez-vous d'avoir installé :
- **CMake** (version 3.15 ou supérieure) : Pour la génération des fichiers de build.
- **Compilateur C++** : Visual Studio 2019/2022 avec MSVC, ou GCC/Clang.
- **vcpkg** : Gestionnaire de paquets C++ pour les dépendances.
- **Git** : Pour le contrôle de version.

### Installation de vcpkg
1. Clonez vcpkg : `git clone https://github.com/Microsoft/vcpkg.git`
2. Exécutez `.\vcpkg\bootstrap-vcpkg.bat` (Windows) ou `./vcpkg/bootstrap-vcpkg.sh` (Linux/Mac).
3. Intégrez vcpkg : `.\vcpkg\vcpkg integrate install` (ajoute à PATH automatiquement).

## Installation et Build

### Étape 1 : Cloner le dépôt
```bash
git clone https://github.com/Kurama73/NihilEngine-Repo.git
cd NihilEngine-Repo
```

### Étape 2 : Installer les dépendances via vcpkg
Les dépendances sont automatiquement gérées via vcpkg. Elles incluent :
- GLFW (fenêtrage)
- GLAD (chargement OpenGL)
- GLM (mathématiques 3D)

Aucune action manuelle requise – CMake les trouve via le toolchain file.

### Étape 3 : Configurer et Build
Utilisez CMake pour configurer et build :

#### Via Terminal
```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build . --config Debug
```

#### Via VSCode
1. Ouvrez le projet dans VSCode.
2. Exécutez la tâche "Configure CMake" (Ctrl+Shift+P > Tasks: Run Task).
3. Build via "Build Debug" (Ctrl+Shift+B).

Le build génère :
- `NihilEngine.lib` : Bibliothèque du moteur.
- `MonJeu.exe` : Exécutable du jeu de test.

## Exécution

### Lancer MonJeu
Après build, exécutez :
```bash
cd build/MonJeu/Debug
./MonJeu.exe
```

Ou via VSCode : Debug > "Debug MonJeu" (F5).

### Contrôles
- **Souris** : Tourner la caméra (yaw/pitch).
- **WASD** : Se déplacer horizontalement (W: avancer, S: reculer, A: gauche, D: droite).
- **Espace** : Monter (Y+).
- **Shift** : Descendre (Y-).
- **Échap** : Fermer la fenêtre.

Actuellement, un triangle orange et un cube rouge sont rendus pour tester le système d'entités flexible.

## Organisation du Projet

Le projet suit une architecture modulaire pour faciliter la maintenance et l'extension :

```
NihilEngine-Repo/
├── NihilEngine/          # Bibliothèque du moteur
│   ├── include/          # Headers publics (.h)
│   ├── src/              # Implémentations (.cpp)
│   └── CMakeLists.txt    # Build de la lib
├── MonJeu/               # Application de test
│   ├── src/              # Code du jeu
│   └── CMakeLists.txt    # Build de l'exe
├── build/                # Dossier de build (ignoré par Git)
├── .vscode/              # Configs VSCode (tasks, launch)
├── CMakeLists.txt        # Config globale
├── .gitignore            # Fichiers à ignorer
└── README.md             # Ce fichier
```

### Règles d'Organisation
- **Séparation Moteur/Jeu** : NihilEngine ne dépend pas de MonJeu. MonJeu utilise uniquement les APIs publiques.
- **Headers/Implémentations** : Tout header dans `include/`, implémentation dans `src/`.
- **Dépendances** : Gérées via vcpkg et CMake `find_package`.
- **Versioning** : Commits fréquents, branches pour features (e.g., `feature/mesh-loading`).
- **Tests** : Ajouter des tests unitaires dans `NihilEngine/tests/` (futur).

## Checklist de Développement

Utilisez cette checklist pour maintenir la qualité et progresser étape par étape :

### ✅ Phase 1 : Setup de Base (Terminé)
- [x] Fenêtre GLFW + OpenGL initialisée
- [x] Bibliothèque NihilEngine compilée
- [x] MonJeu lance sans erreur
- [x] .gitignore configuré
- [x] VSCode tasks/launch configurés

### 🔄 Phase 2 : Rendu 3D (En Cours)
- [x] Classe Renderer avec shaders simples
- [x] Triangle rendu avec perspective
- [x] Classe Camera (position, rotation, matrices)
- [x] Mouvement relatif de la caméra (WASD + souris)
- [x] Ajouter classe Mesh pour modèles 3D
- [x] Charger un cube via manuel
- [ ] Textures de base (diffuse)

### 🔄 Phase 3 : Système d'Entités Flexible
- [x] Classe Entity (position, rotation, mesh)
- [x] Gestion d'entités sans scène rigide (pour KSP, Assetto, Minecraft)
- [ ] Rendu instancié pour performance
- [ ] Lumière de base (directionnelle)

### 🔄 Phase 4 : Physique Adaptative
- [ ] Physique modulaire (Bullet ou simple)
- [ ] Orbites (KSP), véhicules (Assetto), collisions (Minecraft)
- [ ] Simulation temps réel

### 🔄 Phase 5 : Génération de Contenu
- [ ] Algos procéduraux : mondes voxel, pistes, systèmes stellaires
- [ ] Bruit (Perlin) pour terrains
- [ ] Éditeur intégré (futur)

### 🔄 Phase 6 : Fonctionnalités Avancées
- [ ] Audio via OpenAL
- [ ] Physique via Bullet
- [ ] GUI via ImGui
- [ ] Sauvegarde/chargement monde
- [ ] Multi-joueur (futur)

### 📋 Bonnes Pratiques
- [x] Code compilé sans warnings
- [x] Commits atomiques avec messages clairs
- [ ] Tests unitaires pour classes critiques
- [ ] Documentation des APIs (Doxygen futur)
- [ ] Revue de code avant merge
- [ ] Performance : <16ms/frame (60 FPS)

## Contribution

Ce projet est personnel, mais ouvert à l'expérimentation. Pour contribuer :
1. Fork le repo.
2. Créez une branche feature.
3. Testez localement.
4. Ouvrez une PR avec description détaillée.

## Licence

Aucune licence définie pour le moment – projet éducatif.

---

Développé par Kurama73. Pour questions : [GitHub Issues](https://github.com/Kurama73/NihilEngine-Repo/issues).