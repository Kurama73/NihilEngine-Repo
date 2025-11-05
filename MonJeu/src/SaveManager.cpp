// src/SaveManager.cpp
#include <MonJeu/SaveManager.h>
#include <fstream>
#include <iostream>
#include <algorithm>

namespace MonJeu {

SaveManager::SaveManager(const std::string& saveDir)
    : m_SaveDir(saveDir) {
    try {
        std::filesystem::create_directories(m_SaveDir);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Erreur lors de la création du dossier saves: " << e.what() << std::endl;
    }
}

std::unique_ptr<WorldSaveManager> SaveManager::CreateWorld(const std::string& worldName,
                                                          const std::string& displayName,
                                                          unsigned int seed,
                                                          const std::string& description) {
    if (WorldExists(worldName)) {
        std::cerr << "Le monde '" << worldName << "' existe déjà" << std::endl;
        return nullptr;
    }

    auto worldManager = std::make_unique<WorldSaveManager>(worldName, m_SaveDir.string());

    if (!worldManager->CreateWorldDirectory()) {
        return nullptr;
    }

    WorldInfo info;
    info.name = worldName;
    info.displayName = displayName;
    info.seed = seed;
    info.lastModified = std::filesystem::file_time_type::clock::now();
    info.chunkCount = 0;
    info.description = description;

    if (!SaveWorldInfo(worldName, info)) {
        std::cerr << "Erreur lors de la sauvegarde des informations du monde" << std::endl;
        return nullptr;
    }

    return worldManager;
}

std::unique_ptr<WorldSaveManager> SaveManager::LoadWorld(const std::string& worldName) {
    if (!WorldExists(worldName)) {
        std::cerr << "Le monde '" << worldName << "' n'existe pas" << std::endl;
        return nullptr;
    }

    return std::make_unique<WorldSaveManager>(worldName, m_SaveDir.string());
}

std::vector<WorldInfo> SaveManager::ListWorlds() const {
    std::vector<WorldInfo> worlds;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(m_SaveDir)) {
            if (entry.is_directory()) {
                std::string worldName = entry.path().filename().string();

                if (std::filesystem::exists(GetWorldInfoPath(worldName))) {
                    WorldInfo info = LoadWorldInfo(worldName);

                    // Met à jour le nombre de chunks
                    WorldSaveManager tempManager(worldName, m_SaveDir.string());
                    info.chunkCount = tempManager.ListSavedChunks().size();

                    worlds.push_back(info);
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Erreur lors de la liste des mondes: " << e.what() << std::endl;
    }

    // Trie par date de modification (plus récent en premier)
    std::sort(worlds.begin(), worlds.end(), [](const WorldInfo& a, const WorldInfo& b) {
        return a.lastModified > b.lastModified;
    });

    return worlds;
}

bool SaveManager::DeleteWorld(const std::string& worldName) {
    if (!WorldExists(worldName)) {
        return false;
    }

    try {
        std::filesystem::path worldPath = m_SaveDir / worldName;
        std::filesystem::remove_all(worldPath);
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Erreur lors de la suppression du monde: " << e.what() << std::endl;
        return false;
    }
}

bool SaveManager::WorldExists(const std::string& worldName) const {
    std::filesystem::path worldPath = m_SaveDir / worldName;
    return std::filesystem::exists(worldPath) && std::filesystem::is_directory(worldPath);
}

WorldInfo SaveManager::GetWorldInfo(const std::string& worldName) const {
    if (!WorldExists(worldName)) {
        throw std::runtime_error("Le monde n'existe pas");
    }

    WorldInfo info = LoadWorldInfo(worldName);

    // Met à jour le nombre de chunks
    WorldSaveManager tempManager(worldName, m_SaveDir.string());
    info.chunkCount = tempManager.ListSavedChunks().size();

    return info;
}

bool SaveManager::UpdateWorldInfo(const std::string& worldName, const WorldInfo& info) {
    if (!WorldExists(worldName)) {
        return false;
    }

    return SaveWorldInfo(worldName, info);
}

std::filesystem::path SaveManager::GetWorldInfoPath(const std::string& worldName) const {
    return m_SaveDir / worldName / GetWorldInfoFilename();
}

bool SaveManager::SaveWorldInfo(const std::string& worldName, const WorldInfo& info) {
    std::filesystem::path infoPath = GetWorldInfoPath(worldName);

    std::ofstream file(infoPath);
    if (!file) {
        std::cerr << "Erreur: impossible d'ouvrir le fichier info pour écriture: " << infoPath << std::endl;
        return false;
    }

    file << "name=" << info.name << std::endl;
    file << "display_name=" << info.displayName << std::endl;
    file << "seed=" << info.seed << std::endl;
    file << "description=" << info.description << std::endl;
    file << "last_modified=" << std::chrono::duration_cast<std::chrono::seconds>(
        info.lastModified.time_since_epoch()).count() << std::endl;

    return true;
}

WorldInfo SaveManager::LoadWorldInfo(const std::string& worldName) const {
    WorldInfo info;
    info.name = worldName;

    std::filesystem::path infoPath = GetWorldInfoPath(worldName);
    std::ifstream file(infoPath);

    if (!file) {
        throw std::runtime_error("Impossible de lire le fichier info du monde");
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t separator = line.find('=');
        if (separator != std::string::npos) {
            std::string key = line.substr(0, separator);
            std::string value = line.substr(separator + 1);

            if (key == "display_name") {
                info.displayName = value;
            } else if (key == "seed") {
                info.seed = std::stoul(value);
            } else if (key == "description") {
                info.description = value;
            } else if (key == "last_modified") {
                auto timestamp = std::chrono::seconds(std::stoll(value));
                info.lastModified = std::filesystem::file_time_type(timestamp);
            }
        }
    }

    return info;
}

} // namespace MonJeu