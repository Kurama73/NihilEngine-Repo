// src/WorldSaveManager.cpp
#include <MonJeu/WorldSaveManager.h>
#include <fstream>
#include <iostream>
#include <sstream>

namespace MonJeu {

WorldSaveManager::WorldSaveManager(const std::string& worldName, const std::string& saveDir)
    : m_WorldName(worldName) {
    m_WorldPath = std::filesystem::path(saveDir) / worldName;
}

bool WorldSaveManager::CreateWorldDirectory() {
    try {
        std::filesystem::create_directories(m_WorldPath);
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Erreur lors de la création du dossier monde: " << e.what() << std::endl;
        return false;
    }
}

bool WorldSaveManager::SaveChunk(const Chunk& chunk) {
    if (!EnsureChunkDirectory(chunk.GetChunkX(), chunk.GetChunkZ())) {
        return false;
    }

    auto chunkPath = GetChunkPath(chunk.GetChunkX(), chunk.GetChunkZ());
    auto data = ChunkSerializer::SerializeChunk(chunk);

    std::ofstream file(chunkPath, std::ios::binary);
    if (!file) {
        std::cerr << "Erreur: impossible d'ouvrir le fichier pour écriture: " << chunkPath << std::endl;
        return false;
    }

    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    file.close();

    if (!file) {
        std::cerr << "Erreur lors de l'écriture du chunk: " << chunkPath << std::endl;
        return false;
    }

    return true;
}

std::unique_ptr<Chunk> WorldSaveManager::LoadChunk(int chunkX, int chunkZ) {
    auto chunkPath = GetChunkPath(chunkX, chunkZ);

    if (!std::filesystem::exists(chunkPath)) {
        return nullptr;
    }

    std::ifstream file(chunkPath, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Erreur: impossible d'ouvrir le fichier pour lecture: " << chunkPath << std::endl;
        return nullptr;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(size);
    if (!file.read(reinterpret_cast<char*>(data.data()), size)) {
        std::cerr << "Erreur lors de la lecture du chunk: " << chunkPath << std::endl;
        return nullptr;
    }

    try {
        auto serializedChunk = ChunkSerializer::DeserializeChunk(data);
        return ChunkSerializer::CreateChunkFromSerializedData(serializedChunk);
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la désérialisation du chunk: " << e.what() << std::endl;
        return nullptr;
    }
}

bool WorldSaveManager::ChunkExists(int chunkX, int chunkZ) const {
    return std::filesystem::exists(GetChunkPath(chunkX, chunkZ));
}

bool WorldSaveManager::DeleteChunk(int chunkX, int chunkZ) {
    auto chunkPath = GetChunkPath(chunkX, chunkZ);

    try {
        return std::filesystem::remove(chunkPath);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Erreur lors de la suppression du chunk: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::pair<int, int>> WorldSaveManager::ListSavedChunks() const {
    std::vector<std::pair<int, int>> chunks;

    if (!std::filesystem::exists(m_WorldPath)) {
        return chunks;
    }

    // Parcourt tous les fichiers .chunk dans le dossier du monde
    for (const auto& entry : std::filesystem::recursive_directory_iterator(m_WorldPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".chunk") {
            std::string filename = entry.path().stem().string();

            // Le nom de fichier est "chunk_X_Z"
            size_t first_underscore = filename.find('_');
            size_t second_underscore = filename.find('_', first_underscore + 1);

            if (first_underscore != std::string::npos && second_underscore != std::string::npos) {
                try {
                    int chunkX = std::stoi(filename.substr(first_underscore + 1, second_underscore - first_underscore - 1));
                    int chunkZ = std::stoi(filename.substr(second_underscore + 1));
                    chunks.emplace_back(chunkX, chunkZ);
                } catch (const std::exception&) {
                    // Ignore les fichiers malformés
                }
            }
        }
    }

    return chunks;
}

bool WorldSaveManager::SavePlayerState(const PlayerState& playerState) {
    std::filesystem::path playerPath = m_WorldPath / "player.dat";

    std::ofstream file(playerPath, std::ios::binary);
    if (!file) {
        std::cerr << "Erreur: impossible d'ouvrir le fichier player.dat pour écriture: " << playerPath << std::endl;
        return false;
    }

    // Version du format
    uint32_t version = 1;
    file.write(reinterpret_cast<const char*>(&version), sizeof(uint32_t));

    // Position
    file.write(reinterpret_cast<const char*>(&playerState.position.x), sizeof(float));
    file.write(reinterpret_cast<const char*>(&playerState.position.y), sizeof(float));
    file.write(reinterpret_cast<const char*>(&playerState.position.z), sizeof(float));

    // Rotation
    file.write(reinterpret_cast<const char*>(&playerState.yaw), sizeof(float));
    file.write(reinterpret_cast<const char*>(&playerState.pitch), sizeof(float));

    return file.good();
}

bool WorldSaveManager::LoadPlayerState(PlayerState& playerState) {
    std::filesystem::path playerPath = m_WorldPath / "player.dat";

    if (!std::filesystem::exists(playerPath)) {
        return false;
    }

    std::ifstream file(playerPath, std::ios::binary);
    if (!file) {
        std::cerr << "Erreur: impossible d'ouvrir le fichier player.dat pour lecture: " << playerPath << std::endl;
        return false;
    }

    // Version du format
    uint32_t version;
    file.read(reinterpret_cast<char*>(&version), sizeof(uint32_t));

    if (version != 1) {
        std::cerr << "Version du fichier player.dat non supportée: " << version << std::endl;
        return false;
    }

    // Position
    file.read(reinterpret_cast<char*>(&playerState.position.x), sizeof(float));
    file.read(reinterpret_cast<char*>(&playerState.position.y), sizeof(float));
    file.read(reinterpret_cast<char*>(&playerState.position.z), sizeof(float));

    // Rotation
    file.read(reinterpret_cast<char*>(&playerState.yaw), sizeof(float));
    file.read(reinterpret_cast<char*>(&playerState.pitch), sizeof(float));

    return file.good();
}

bool WorldSaveManager::PlayerStateExists() const {
    return std::filesystem::exists(m_WorldPath / "player.dat");
}

std::string WorldSaveManager::GetChunkFilename(int chunkX, int chunkZ) const {
    std::stringstream ss;
    ss << "chunk_" << chunkX << "_" << chunkZ << ".chunk";
    return ss.str();
}

std::filesystem::path WorldSaveManager::GetChunkPath(int chunkX, int chunkZ) const {
    // Organise les chunks dans des sous-dossiers pour éviter trop de fichiers dans un seul dossier
    int folderX = chunkX / 32;  // 32 chunks par dossier
    int folderZ = chunkZ / 32;

    std::filesystem::path chunkDir = m_WorldPath / std::to_string(folderX) / std::to_string(folderZ);
    return chunkDir / GetChunkFilename(chunkX, chunkZ);
}

bool WorldSaveManager::EnsureChunkDirectory(int chunkX, int chunkZ) {
    std::filesystem::path chunkDir = GetChunkPath(chunkX, chunkZ).parent_path();

    try {
        std::filesystem::create_directories(chunkDir);
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Erreur lors de la création du dossier chunk: " << e.what() << std::endl;
        return false;
    }
}

} // namespace MonJeu