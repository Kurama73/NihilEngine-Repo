#pragma once

#include <glm/glm.hpp>
#include <functional>
#include <limits> // Ajouté pour std::numeric_limits

namespace NihilEngine {

    struct RaycastHit {
        bool hit = false;
        glm::ivec3 blockPosition; // Coordonnée du bloc touché
        glm::vec3 hitNormal;       // Normale de la face touchée
        glm::vec3 hitPoint;        // <-- NOUVEAU: Le point d'impact exact (flottant)
    };

    /**
     * @brief Traverse une grille de voxels le long d'un rayon.
     * @param origin Point de départ du rayon (ex: caméra)
     * @param direction Direction normalisée du rayon
     * @param maxDistance Distance maximale
     * @param voxelCheck Fonction callback que le moteur appelle pour chaque case.
     * Doit retourner TRUE s'il y a un bloc, FALSE sinon.
     * @param outHit Structure pour stocker le résultat.
     * @return true si un bloc a été touché, false sinon.
     */
    static bool RaycastVoxel(
        const glm::vec3& origin,
        const glm::vec3& direction,
        float maxDistance,
        std::function<bool(const glm::ivec3&)> voxelCheck, // Callback
        RaycastHit& outHit
    ) {
        glm::ivec3 currentVoxel = glm::floor(origin);
        glm::vec3 tMax; // Distance pour atteindre le prochain plan (X, Y, Z)
        glm::vec3 tDelta; // Distance pour traverser 1 voxel (X, Y, Z)
        glm::ivec3 step;   // Direction du pas (1, -1)

        for (int i = 0; i < 3; ++i) {
            if (direction[i] > 0.0f) {
                step[i] = 1;
                tMax[i] = (currentVoxel[i] + 1 - origin[i]) / direction[i];
                tDelta[i] = 1.0f / direction[i];
            } else if (direction[i] < 0.0f) {
                step[i] = -1;
                // --- CORRECTION DE BUG ---
                // Votre formule (origin[i] - currentVoxel[i]) donnait un t négatif
                tMax[i] = (currentVoxel[i] - origin[i]) / direction[i];
                tDelta[i] = -1.0f / direction[i]; // C'est correct (négatif / négatif = positif)
            } else {
                step[i] = 0;
                tMax[i] = std::numeric_limits<float>::infinity();
                tDelta[i] = std::numeric_limits<float>::infinity();
            }
        }

        float distance = 0.0f;
        glm::vec3 lastNormal(0.0f); // Stocker la dernière normale

        while (distance < maxDistance) {
            // 1. Vérifier le bloc courant
            // (On suppose qu'on commence hors d'un bloc, ce qui est géré par l'offset dans main.cpp)
            if (voxelCheck(currentVoxel)) {
                outHit.hit = true;
                outHit.blockPosition = currentVoxel;

                // --- AMÉLIORATION ---
                // On stocke la normale et le point d'impact de la face qu'on vient de traverser
                outHit.hitNormal = lastNormal;
                outHit.hitPoint = origin + direction * distance; // 'distance' est le t de l'impact

                return true;
            }

            // 2. Avancer au prochain voxel
            int side; // 0=X, 1=Y, 2=Z
            if (tMax.x < tMax.y) {
                if (tMax.x < tMax.z) {
                    side = 0;
                    distance = tMax.x;
                } else {
                    side = 2;
                    distance = tMax.z;
                }
            } else {
                if (tMax.y < tMax.z) {
                    side = 1;
                    distance = tMax.y;
                } else {
                    side = 2;
                    distance = tMax.z;
                }
            }

            if (distance > maxDistance) break;

            // --- CORRECTION LOGIQUE ---
            // On stocke la normale de la face qu'on s'apprête à traverser
            lastNormal = glm::vec3(0.0f);
            lastNormal[side] = -step[side];

            // On avance au voxel suivant
            currentVoxel[side] += step[side];
            tMax[side] += tDelta[side];

            // (L'ancienne 'outHit.hitNormal = ...' a été déplacée)
        }

        outHit.hit = false;
        return false;
    }
}