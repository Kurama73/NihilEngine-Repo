#include <NihilEngine/EntityController.h>
#include <NihilEngine/Camera.h>
#include <NihilEngine/Input.h>
#include <algorithm>

namespace NihilEngine {

    EntityController::EntityController(Camera& camera, int switchKey)
        : m_Camera(camera), m_SwitchKey(switchKey)
    {
    }

    void EntityController::AddControllableEntity(IControllableEntity* entity) {
        if (entity && std::find(m_Entities.begin(), m_Entities.end(), entity) == m_Entities.end()) {
            m_Entities.push_back(entity);
        }
    }

    void EntityController::RemoveControllableEntity(IControllableEntity* entity) {
        auto it = std::find(m_Entities.begin(), m_Entities.end(), entity);
        if (it != m_Entities.end()) {
            m_Entities.erase(it);
            if (m_CurrentIndex >= m_Entities.size() && m_Entities.size() > 0) {
                m_CurrentIndex = m_Entities.size() - 1;
            }
        }
    }

    void EntityController::ClearEntities() {
        m_Entities.clear();
        m_CurrentIndex = 0;
    }

    void EntityController::Update(float deltaTime, MonJeu::VoxelWorld& world) {
        if (m_Entities.empty()) return;

        if (Input::IsKeyTriggered(m_SwitchKey)) {
            SwitchToNextEntity();
        }

        m_Entities[m_CurrentIndex]->Update(deltaTime, m_Camera, world, true);
    }

    void EntityController::Render(Renderer& renderer, const Camera& camera, bool firstPerson) {
        if (m_Entities.empty()) return;

        for (size_t i = 0; i < m_Entities.size(); ++i) {
            if (!firstPerson || i != m_CurrentIndex) {
                m_Entities[i]->Render(renderer, camera);
            }
        }
    }

    void EntityController::SwitchToNextEntity() {
        if (m_Entities.size() <= 1) return;

        m_CurrentIndex = (m_CurrentIndex + 1) % m_Entities.size();
        auto* current = m_Entities[m_CurrentIndex];

        m_Camera.SetRotation(current->GetYaw(), current->GetPitch());
    }

    IControllableEntity* EntityController::GetCurrentEntity() const {
        return m_Entities.empty() ? nullptr : m_Entities[m_CurrentIndex];
    }

    size_t EntityController::GetEntityCount() const {
        return m_Entities.size();
    }

    int EntityController::GetCurrentIndex() const {
        return static_cast<int>(m_CurrentIndex);
    }

}
