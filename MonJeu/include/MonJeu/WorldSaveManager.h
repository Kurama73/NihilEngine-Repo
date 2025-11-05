// include/MonJeu/WorldSaveManager.h
#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>
#include <memory>
#include "ChunkSerializer.h"

namespace MonJeu {

/**
 * @brief Informations sur l'état du joueur et de la caméra
 */
struct PlayerState {
    glm::vec3 position;
    float yaw;
    float pitch;
    // Peut être étendu pour d'autres propriétés du joueur
};

/**
 * @brief Gère la sauvegarde et le chargement des chunks pour un monde spécifique.
 * Utilise un système de fichiers avec des fichiers par chunk pour la performance.
 */
class WorldSaveManager {
public:
    /**
     * @brief Constructeur
     * @param worldName Nom du monde (utilisé pour créer le dossier de sauvegarde)
     * @param saveDir Répertoire de base pour les sauvegardes
     */
    WorldSaveManager(const std::string& worldName, const std::string& saveDir = "saves");

    /**
     * @brief Sauvegarde un chunk sur le disque
     */
    bool SaveChunk(const Chunk& chunk);

    /**
     * @brief Charge un chunk depuis le disque
     * @return nullptr si le chunk n'existe pas
     */
    std::unique_ptr<Chunk> LoadChunk(int chunkX, int chunkZ);

    /**
     * @brief Vérifie si un chunk existe sur le disque
     */
    bool ChunkExists(int chunkX, int chunkZ) const;

    /**
     * @brief Supprime un chunk du disque
     */
    bool DeleteChunk(int chunkX, int chunkZ);

    /**
     * @brief Liste tous les chunks sauvegardés
     * @return Vecteur de paires (chunkX, chunkZ)
     */
    std::vector<std::pair<int, int>> ListSavedChunks() const;

    /**
     * @brief Obtient le chemin du dossier du monde
     */
    std::filesystem::path GetWorldPath() const { return m_WorldPath; }

    /**
     * @brief Crée le dossier du monde s'il n'existe pas
     */
    bool CreateWorldDirectory();

    /**
     * @brief Sauvegarde l'état du joueur
     */
    bool SavePlayerState(const PlayerState& playerState);

    /**
     * @brief Charge l'état du joueur
     * @return true si l'état a été chargé, false sinon
     */
    bool LoadPlayerState(PlayerState& playerState);

    /**
     * @brief Vérifie si l'état du joueur existe
     */
    bool PlayerStateExists() const;

private:
    std::filesystem::path m_WorldPath;
    std::string m_WorldName;

    /**
     * @brief Génère le nom de fichier pour un chunk
     */
    std::string GetChunkFilename(int chunkX, int chunkZ) const;

    /**
     * @brief Génère le chemin complet pour un chunk
     */
    std::filesystem::path GetChunkPath(int chunkX, int chunkZ) const;

    /**
     * @brief Crée les dossiers nécessaires pour un chunk
     */
    bool EnsureChunkDirectory(int chunkX, int chunkZ);
};

} // namespace MonJeu