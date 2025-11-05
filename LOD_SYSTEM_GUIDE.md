# Système LOD (Level of Detail) - Guide d'utilisation

## Vue d'ensemble

Le système LOD implémente une gestion optimisée des chunks basée sur la distance, inspirée de Distant Horizons. Il permet de gérer efficacement de vastes mondes procéduraux en ajustant automatiquement le niveau de détail des chunks selon leur distance par rapport à la caméra.

## Composants du système

### 1. LODSystem
- **Rôle** : Détermine le niveau de détail approprié pour chaque chunk selon la distance
- **Niveaux de LOD** :
  - `HIGH_DETAIL` : Chunks proches avec détails complets
  - `MEDIUM_DETAIL` : Chunks moyens avec détails réduits
  - `LOW_DETAIL` : Chunks lointains avec détails simplifiés
  - `VERY_LOW_DETAIL` : Chunks très lointains avec données minimales

### 2. ChunkDataCache
- **Rôle** : Stocke les données simplifiées des chunks distants
- **Données cachées** :
  - Hauteurs moyennes, minimales et maximales
  - Biome dominant
  - Couleur représentative
  - Échelle du chunk

### 3. ProgressiveChunkUpdate
- **Rôle** : Met à jour les chunks de manière progressive pour éviter les pics de performance
- **Fonctionnalités** :
  - File de priorité pour les mises à jour
  - Limitation du nombre de mises à jour par frame
  - Annulation des demandes obsolètes

### 4. LODMeshGenerator
- **Rôle** : Génère des meshes adaptés à chaque niveau de LOD
- **Génération** :
  - Meshes détaillés pour les chunks proches
  - Meshes simplifiés (plans) pour les chunks distants

## Configuration

### Paramètres LOD
```cpp
NihilEngine::LODSettings lodSettings;
lodSettings.calculationDistance = 64.0f;  // Distance de calcul des entités (chunks)
lodSettings.displayDistance = 32.0f;     // Distance d'affichage (chunks)
lodSettings.generationDistance = 48.0f;  // Distance de génération (chunks)
```

### Distances par défaut
- **Calculation** : 64 chunks - Les entités dans ces chunks sont mises à jour
- **Display** : 32 chunks - Ces chunks sont rendus avec leur LOD approprié
- **Generation** : 48 chunks - Ces chunks sont générés/maintenus en mémoire

## Utilisation dans VoxelWorld

### Initialisation
Le système LOD est automatiquement initialisé dans le constructeur de `VoxelWorld` avec les paramètres par défaut.

### Mise à jour
La mise à jour LOD est appelée automatiquement dans `VoxelWorld::Render()` :
```cpp
void VoxelWorld::Render(NihilEngine::Renderer& renderer, const NihilEngine::Camera& camera) {
    // Mise à jour automatique du système LOD
    UpdateLOD(camera.GetPosition(), deltaTime);
    // ... rendu des chunks
}
```

### Génération LOD
Pour générer un chunk avec un LOD spécifique :
```cpp
void VoxelWorld::GenerateChunkLOD(int chunkX, int chunkZ, NihilEngine::LODLevel lodLevel) {
    if (lodLevel == NihilEngine::LODLevel::HIGH_DETAIL) {
        // Génération complète du chunk voxel
        GenerateChunk(chunkX, chunkZ);
    } else {
        // Génération de données simplifiées et mesh LOD
        // ...
    }
}
```

## Optimisations

### Performance
- **Mises à jour progressives** : Maximum 2 chunks mis à jour par frame
- **Cache intelligent** : Données simplifiées pour chunks distants
- **LOD adaptatif** : Ajustement automatique selon la distance

### Mémoire
- **Chunks distants** : Utilisent des meshes simplifiés (4 vertices au lieu de milliers)
- **Nettoyage automatique** : Suppression des données anciennes (>5 minutes)
- **Limitation** : Maximum 100 mises à jour en attente

## Avantages

1. **Performance constante** : Pas de pics lors du chargement de nouveaux chunks
2. **Évolutivité** : Supporte des mondes très vastes
3. **Transitions fluides** : Changements de LOD progressifs
4. **Optimisation mémoire** : Chunks distants utilisent peu de ressources
5. **Calculs distribués** : Entités actives même dans chunks non affichés

## Limitations actuelles

- Le système LOD est intégré à VoxelWorld mais pourrait être étendu à d'autres types de terrains
- Les transitions LOD sont instantanées (pas de blending morphing)
- Le cache utilise une approche simple (pas de compression avancée)

## Extension future

- **Blending LOD** : Transitions morphing entre niveaux de détail
- **LOD pour entités** : Système LOD pour les objets dynamiques
- **Compression** : Compression des données de cache
- **Préchargement** : Chargement prédictif selon le mouvement du joueur