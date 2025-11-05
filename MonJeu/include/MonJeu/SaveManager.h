// include/MonJeu/SaveManager.h
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include "WorldSaveManager.h"

namespace MonJeu {

/**
 * @brief Informations sur un monde sauvegardé
 */
struct WorldInfo {
    std::string name;
    std::string displayName;
    unsigned int seed;
    std::filesystem::file_time_type lastModified;
    size_t chunkCount;
    std::string description;
};

/**
 * @brief Gère la création, la liste et la suppression des mondes sauvegardés.
 */
class SaveManager {
public:
    /**
     * @brief Constructeur
     * @param saveDir Répertoire de base pour les sauvegardes
     */
    SaveManager(const std::string& saveDir = "saves");

    /**
     * @brief Crée un nouveau monde
     * @param worldName Nom unique du monde
     * @param displayName Nom d'affichage
     * @param seed Seed du monde
     * @param description Description optionnelle
     * @return Pointeur vers le WorldSaveManager du nouveau monde, nullptr en cas d'erreur
     */
    std::unique_ptr<WorldSaveManager> CreateWorld(const std::string& worldName,
                                                  const std::string& displayName,
                                                  unsigned int seed,
                                                  const std::string& description = "");

    /**
     * @brief Charge un monde existant
     * @param worldName Nom du monde
     * @return Pointeur vers le WorldSaveManager du monde, nullptr si le monde n'existe pas
     */
    std::unique_ptr<WorldSaveManager> LoadWorld(const std::string& worldName);

    /**
     * @brief Liste tous les mondes disponibles
     */
    std::vector<WorldInfo> ListWorlds() const;

    /**
     * @brief Supprime un monde
     */
    bool DeleteWorld(const std::string& worldName);

    /**
     * @brief Vérifie si un monde existe
     */
    bool WorldExists(const std::string& worldName) const;

    /**
     * @brief Obtient les informations d'un monde
     */
    WorldInfo GetWorldInfo(const std::string& worldName) const;

    /**
     * @brief Met à jour les informations d'un monde
     */
    bool UpdateWorldInfo(const std::string& worldName, const WorldInfo& info);

private:
    std::filesystem::path m_SaveDir;

    /**
     * @brief Nom du fichier d'informations du monde
     */
    std::string GetWorldInfoFilename() const { return "world.info"; }

    /**
     * @brief Chemin du fichier d'informations d'un monde
     */
    std::filesystem::path GetWorldInfoPath(const std::string& worldName) const;

    /**
     * @brief Sauvegarde les informations d'un monde
     */
    bool SaveWorldInfo(const std::string& worldName, const WorldInfo& info);

    /**
     * @brief Charge les informations d'un monde
     */
    WorldInfo LoadWorldInfo(const std::string& worldName) const;
};

} // namespace MonJeu