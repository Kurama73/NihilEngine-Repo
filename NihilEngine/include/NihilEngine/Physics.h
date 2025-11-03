#pragma once

#include <glm/glm.hpp>
#include <functional>

namespace NihilEngine {

    struct RaycastHit {
        bool hit = false;
        glm::ivec3 blockPosition; // Coordonnée du bloc touché
        glm::vec3 hitNormal;      // Normale de la face touchée
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
                tMax[i] = (origin[i] - currentVoxel[i]) / direction[i];
                tDelta[i] = -1.0f / direction[i];
            } else {
                step[i] = 0;
                tMax[i] = std::numeric_limits<float>::infinity();
                tDelta[i] = std::numeric_limits<float>::infinity();
            }
        }

        float distance = 0.0f;
        while (distance < maxDistance) {
            // 1. Vérifier le bloc courant
            if (voxelCheck(currentVoxel)) {
                outHit.hit = true;
                outHit.blockPosition = currentVoxel;
                // (Calcul de la normale omis pour la simplicité,
                // on peut le déduire du 'side' qui a été touché)
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

            currentVoxel[side] += step[side];
            tMax[side] += tDelta[side];

            // Calculer la normale (simple)
            outHit.hitNormal = glm::vec3(0.0f);
            outHit.hitNormal[side] = -step[side];
        }

        outHit.hit = false;
        return false;
    }
}