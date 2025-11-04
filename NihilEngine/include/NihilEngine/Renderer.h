// NihilEngine/include/NihilEngine/Renderer.h
#pragma once
#include <NihilEngine/Entity.h>
#include <NihilEngine/Camera.h>
#include <glm/glm.hpp>

namespace NihilEngine {

class Renderer {
public:
    Renderer();
    ~Renderer();

    void Clear();
    void DrawEntity(const Entity& entity, const Camera& camera);
    void DrawCrosshair(int windowWidth, int windowHeight);
    void DrawLine3D(const glm::vec3& start, const glm::vec3& end, const Camera& camera, const glm::vec3& color, float width = 1.0f);
    void DrawWireCube(const glm::vec3& min, const glm::vec3& max, const Camera& camera, const glm::vec3& color);
    void DrawCube(const glm::mat4& model, const glm::vec4& color, const Camera& camera);
private:
    void InitShaders();
    void InitCrosshair();
    void InitLineShader();

    unsigned int m_ShaderProgram = 0;
    unsigned int m_CrosshairShaderProgram = 0;
    unsigned int m_LineShaderProgram = 0;
    unsigned int m_CrosshairVAO = 0, m_CrosshairVBO = 0;
    unsigned int m_LineVAO = 0, m_LineVBO = 0;
};

} // namespace NihilEngine