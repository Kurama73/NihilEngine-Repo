// Renderer.cpp
#include <NihilEngine/Renderer.h>
#include <NihilEngine/Camera.h>
#include <NihilEngine/Entity.h>
#include <NihilEngine/Constants.h>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace NihilEngine {

    // ============================================================
    // SHADERS
    // ============================================================

    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aUv;

        out vec3 v_FragPos;
        out vec3 v_Normal;
        out vec2 v_Uv;

        uniform mat4 u_Model;
        uniform mat4 u_ViewProjection;

        void main() {
            vec4 worldPos = u_Model * vec4(aPos, 1.0);
            gl_Position = u_ViewProjection * worldPos;
            v_FragPos = worldPos.xyz;
            v_Normal = mat3(transpose(inverse(u_Model))) * aNormal;
            v_Uv = aUv;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec3 v_FragPos;
        in vec3 v_Normal;
        in vec2 v_Uv;
        out vec4 FragColor;

        uniform vec3 u_ViewPos;
        uniform sampler2D u_Texture;
        uniform vec4 u_Color;
        uniform bool u_HasTexture;
        uniform bool u_FogEnabled;
        uniform vec3 u_FogColor;
        uniform float u_FogDensity;
        uniform vec3 u_LightPos;

        void main() {
            vec4 baseColor = u_Color;
            if (u_HasTexture) {
                baseColor *= texture(u_Texture, v_Uv);
            }

            // Ambient
            float ambientStrength = 0.3;
            vec3 ambient = ambientStrength * vec3(1.0);

            // Diffuse
            vec3 norm = normalize(v_Normal);
            vec3 lightDir = normalize(u_LightPos - v_FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * vec3(1.0);

            vec3 result = (ambient + diffuse) * baseColor.rgb;

            if (u_FogEnabled) {
                float distance = length(v_FragPos - u_ViewPos);
                float fogFactor = 1.0 - exp(-u_FogDensity * distance);
                result = mix(result, u_FogColor, fogFactor);
            }

            FragColor = vec4(result, baseColor.a);
        }
    )";

    // Crosshair
    const char* crosshairVertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        uniform mat4 u_Projection;
        uniform mat4 u_Model;
        void main() {
            gl_Position = u_Projection * u_Model * vec4(aPos, 0.0, 1.0);
        }
    )";

    const char* crosshairFragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        }
    )";

    // Line 3D
    const char* lineVertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        uniform mat4 u_ViewProjection;
        void main() {
            gl_Position = u_ViewProjection * vec4(aPos, 1.0);
        }
    )";

    const char* lineFragmentShaderSource = R"(
        #version 330 core
        uniform vec3 u_Color;
        out vec4 FragColor;
        void main() {
            FragColor = vec4(u_Color, 1.0);
        }
    )";

    // ============================================================
    // RENDERER IMPLEMENTATION
    // ============================================================

    Renderer::Renderer() {
        glEnable(GL_DEPTH_TEST);
        InitShaders();
        InitCrosshair();
        InitLineShader();
        std::cout << "Renderer initialized\n";
    }

    Renderer::~Renderer() {
        glDeleteProgram(m_ShaderProgram);
        glDeleteProgram(m_CrosshairShaderProgram);
        glDeleteProgram(m_LineShaderProgram);

        glDeleteVertexArrays(1, &m_CrosshairVAO);
        glDeleteBuffers(1, &m_CrosshairVBO);
        glDeleteVertexArrays(1, &m_LineVAO);
        glDeleteBuffers(1, &m_LineVBO);
    }

    void Renderer::InitShaders() {
        GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vertexShaderSource, nullptr);
        glCompileShader(vertex);

        GLint success;
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
            std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
        }

        GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragment);

        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
            std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
        }

        m_ShaderProgram = glCreateProgram();
        glAttachShader(m_ShaderProgram, vertex);
        glAttachShader(m_ShaderProgram, fragment);
        glLinkProgram(m_ShaderProgram);

        glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(m_ShaderProgram, 512, nullptr, infoLog);
            std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        }

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    void Renderer::InitCrosshair() {
        GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &crosshairVertexShaderSource, nullptr);
        glCompileShader(vertex);

        GLint success;
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
            std::cerr << "Crosshair vertex shader compilation failed: " << infoLog << std::endl;
        }

        GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &crosshairFragmentShaderSource, nullptr);
        glCompileShader(fragment);

        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
            std::cerr << "Crosshair fragment shader compilation failed: " << infoLog << std::endl;
        }

        m_CrosshairShaderProgram = glCreateProgram();
        glAttachShader(m_CrosshairShaderProgram, vertex);
        glAttachShader(m_CrosshairShaderProgram, fragment);
        glLinkProgram(m_CrosshairShaderProgram);

        glGetProgramiv(m_CrosshairShaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(m_CrosshairShaderProgram, 512, nullptr, infoLog);
            std::cerr << "Crosshair shader program linking failed: " << infoLog << std::endl;
        }

        glDeleteShader(vertex);
        glDeleteShader(fragment);

        float size = Constants::CROSSHAIR_SIZE;
        std::vector<float> vertices = {
            -size, 0.0f,
            size, 0.0f,
            0.0f, -size,
            0.0f,  size
        };

        glGenVertexArrays(1, &m_CrosshairVAO);
        glGenBuffers(1, &m_CrosshairVBO);
        glBindVertexArray(m_CrosshairVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_CrosshairVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    void Renderer::InitLineShader() {
        GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &lineVertexShaderSource, nullptr);
        glCompileShader(vertex);

        GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &lineFragmentShaderSource, nullptr);
        glCompileShader(fragment);

        m_LineShaderProgram = glCreateProgram();
        glAttachShader(m_LineShaderProgram, vertex);
        glAttachShader(m_LineShaderProgram, fragment);
        glLinkProgram(m_LineShaderProgram);

        glDeleteShader(vertex);
        glDeleteShader(fragment);

        glGenVertexArrays(1, &m_LineVAO);
        glGenBuffers(1, &m_LineVBO);
        glBindVertexArray(m_LineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_LineVBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    void Renderer::Clear() {
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::DrawEntity(const Entity& entity, const Camera& camera) {
        glUseProgram(m_ShaderProgram);

        glm::mat4 vp = camera.GetViewProjectionMatrix();
        glUniformMatrix4fv(glGetUniformLocation(m_ShaderProgram, "u_ViewProjection"), 1, GL_FALSE, glm::value_ptr(vp));
        glUniformMatrix4fv(glGetUniformLocation(m_ShaderProgram, "u_Model"), 1, GL_FALSE, glm::value_ptr(entity.GetModelMatrix()));
        glUniform3fv(glGetUniformLocation(m_ShaderProgram, "u_ViewPos"), 1, glm::value_ptr(camera.GetPosition()));

        // Set lighting uniforms
        glUniform3f(glGetUniformLocation(m_ShaderProgram, "u_LightPos"), Constants::LIGHT_POS_X, Constants::LIGHT_POS_Y, Constants::LIGHT_POS_Z);

        // Set fog uniforms
        glUniform1i(glGetUniformLocation(m_ShaderProgram, "u_FogEnabled"), m_FogEnabled);
        glUniform3fv(glGetUniformLocation(m_ShaderProgram, "u_FogColor"), 1, glm::value_ptr(m_FogColor));
        glUniform1f(glGetUniformLocation(m_ShaderProgram, "u_FogDensity"), m_FogDensity);

        const auto& material = entity.GetMaterial();
        glUniform4fv(glGetUniformLocation(m_ShaderProgram, "u_Color"), 1, glm::value_ptr(material.color));
        bool hasTexture = material.textureID.has_value();
        glUniform1i(glGetUniformLocation(m_ShaderProgram, "u_HasTexture"), hasTexture);

        if (hasTexture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, material.textureID.value());
            glUniform1i(glGetUniformLocation(m_ShaderProgram, "u_Texture"), 0);
        }

        entity.GetMesh().Bind();
        entity.GetMesh().Draw();
        entity.GetMesh().Unbind();

        if (hasTexture) {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    void Renderer::DrawCrosshair(int windowWidth, int windowHeight) {
        glDisable(GL_DEPTH_TEST);
        glUseProgram(m_CrosshairShaderProgram);

        glm::mat4 proj = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(windowWidth / 2.0f, windowHeight / 2.0f, 0.0f));

        glUniformMatrix4fv(glGetUniformLocation(m_CrosshairShaderProgram, "u_Projection"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(m_CrosshairShaderProgram, "u_Model"), 1, GL_FALSE, glm::value_ptr(model));

        glLineWidth(Constants::LINE_WIDTH_DEFAULT);
        glBindVertexArray(m_CrosshairVAO);
        glDrawArrays(GL_LINES, 0, 4);
        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);
    }

    void Renderer::DrawLine3D(const glm::vec3& start, const glm::vec3& end, const Camera& camera, const glm::vec3& color, float width) {
        glDisable(GL_DEPTH_TEST);
        glUseProgram(m_LineShaderProgram);

        glm::mat4 vp = camera.GetViewProjectionMatrix();
        glUniformMatrix4fv(glGetUniformLocation(m_LineShaderProgram, "u_ViewProjection"), 1, GL_FALSE, glm::value_ptr(vp));
        glUniform3fv(glGetUniformLocation(m_LineShaderProgram, "u_Color"), 1, glm::value_ptr(color));

        float vertices[] = {
            start.x, start.y, start.z,
            end.x,   end.y,   end.z
        };

        glBindVertexArray(m_LineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_LineVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        glLineWidth(width);
        glDrawArrays(GL_LINES, 0, 2);
        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);
    }

    void Renderer::DrawWireCube(const glm::vec3& min, const glm::vec3& max, const Camera& camera, const glm::vec3& color) {
        glDisable(GL_DEPTH_TEST);
        glUseProgram(m_LineShaderProgram);

        glm::mat4 vp = camera.GetViewProjectionMatrix();
        glUniformMatrix4fv(glGetUniformLocation(m_LineShaderProgram, "u_ViewProjection"), 1, GL_FALSE, glm::value_ptr(vp));
        glUniform3fv(glGetUniformLocation(m_LineShaderProgram, "u_Color"), 1, glm::value_ptr(color));

        glm::vec3 corners[8] = {
            glm::vec3(min.x, min.y, min.z),
            glm::vec3(max.x, min.y, min.z),
            glm::vec3(max.x, max.y, min.z),
            glm::vec3(min.x, max.y, min.z),
            glm::vec3(min.x, min.y, max.z),
            glm::vec3(max.x, min.y, max.z),
            glm::vec3(max.x, max.y, max.z),
            glm::vec3(min.x, max.y, max.z)
        };

        // 12 edges of the cube
        glm::vec3 edges[24] = {
            corners[0], corners[1], corners[1], corners[2], corners[2], corners[3], corners[3], corners[0], // bottom face
            corners[4], corners[5], corners[5], corners[6], corners[6], corners[7], corners[7], corners[4], // top face
            corners[0], corners[4], corners[1], corners[5], corners[2], corners[6], corners[3], corners[7]  // vertical edges
        };

        glBindVertexArray(m_LineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_LineVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(edges), edges, GL_DYNAMIC_DRAW);
        glLineWidth(Constants::LINE_WIDTH_DEFAULT);
        glDrawArrays(GL_LINES, 0, 24);
        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);
    }

    void Renderer::DrawCube(const glm::mat4& model, const glm::vec4& color, const Camera& camera) {
        glUseProgram(m_ShaderProgram);

        glm::mat4 vp = camera.GetViewProjectionMatrix();
        glUniformMatrix4fv(glGetUniformLocation(m_ShaderProgram, "u_ViewProjection"), 1, GL_FALSE, glm::value_ptr(vp));
        glUniformMatrix4fv(glGetUniformLocation(m_ShaderProgram, "u_Model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(glGetUniformLocation(m_ShaderProgram, "u_ViewPos"), 1, glm::value_ptr(camera.GetPosition()));
        glUniform4fv(glGetUniformLocation(m_ShaderProgram, "u_Color"), 1, glm::value_ptr(color));
        glUniform1i(glGetUniformLocation(m_ShaderProgram, "u_HasTexture"), false);
    }

    // Fog methods
    void Renderer::EnableFog(bool enable) {
        m_FogEnabled = enable;
    }

    void Renderer::SetFogColor(const glm::vec3& color) {
        m_FogColor = color;
    }

    void Renderer::SetFogDensity(float density) {
        m_FogDensity = density;
    }

    // Particle methods
    void Renderer::AddParticle(const Particle& particle) {
        m_Particles.push_back(particle);
    }

    void Renderer::UpdateParticles(float deltaTime) {
        for (auto it = m_Particles.begin(); it != m_Particles.end(); ) {
            it->life -= deltaTime;
            if (it->life <= 0.0f) {
                it = m_Particles.erase(it);
            } else {
                it->position += it->velocity * deltaTime;
                ++it;
            }
        }
    }

    void Renderer::DrawParticles(const Camera& camera) {
        if (m_Particles.empty()) return;

        // TODO: Implement particle rendering with billboarding
        // This would require a particle shader and proper vertex setup
    }

}