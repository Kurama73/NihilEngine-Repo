// NihilEngine/include/NihilEngine/Renderer.h
#pragma once
#include <NihilEngine/Entity.h>
#include <NihilEngine/Camera.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

namespace NihilEngine {

    struct Particle {
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec4 color;
        float life;
        float size;
    };

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

        // Fog
        void EnableFog(bool enable);
        void SetFogColor(const glm::vec3& color);
        void SetFogDensity(float density);

        // Particles
        void AddParticle(const Particle& particle);
        void UpdateParticles(float deltaTime);
        void DrawParticles(const Camera& camera);

    private:
        struct MainUniformLocations {
            GLint viewProjection = -1;
            GLint model = -1;
            GLint viewPos = -1;
            GLint texture = -1;
            GLint color = -1;
            GLint hasTexture = -1;
            GLint fogEnabled = -1;
            GLint fogColor = -1;
            GLint fogDensity = -1;
            GLint lightPos = -1;
        };

        void InitShaders();
        void InitCrosshair();
        void InitLineShader();
        void InitParticleShader();
        void CacheMainUniformLocations();

        unsigned int m_ShaderProgram = 0;
        unsigned int m_CrosshairShaderProgram = 0;
        unsigned int m_LineShaderProgram = 0;
        unsigned int m_ParticleShaderProgram = 0;
        unsigned int m_CrosshairVAO = 0, m_CrosshairVBO = 0;
        unsigned int m_LineVAO = 0, m_LineVBO = 0;
        unsigned int m_ParticleVAO = 0, m_ParticleVBO = 0;
        MainUniformLocations m_MainUniforms;

        // Fog
        bool m_FogEnabled = false;
        glm::vec3 m_FogColor = glm::vec3(0.5f, 0.5f, 0.5f);
        float m_FogDensity = 0.01f;

        // Particles
        std::vector<Particle> m_Particles;
    };

}