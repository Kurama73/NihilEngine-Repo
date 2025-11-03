#include <NihilEngine/Renderer.h>
#include <NihilEngine/Camera.h>
#include <NihilEngine/Mesh.h>
#include <iostream>
#include <vector>

namespace NihilEngine {

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
            gl_Position = u_ViewProjection * u_Model * vec4(aPos, 1.0);
            v_FragPos = vec3(u_Model * vec4(aPos, 1.0));
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
        uniform vec3 u_LightPos = vec3(20, 50, 20);
        uniform vec3 u_ViewPos;
        void main() {
            vec3 blockColor = vec3(0.7, 0.7, 0.7);
            float ambientStrength = 0.3;
            vec3 ambient = ambientStrength * vec3(1.0);
            vec3 norm = normalize(v_Normal);
            vec3 lightDir = normalize(u_LightPos - v_FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * vec3(1.0);
            vec3 result = (ambient + diffuse) * blockColor;
            FragColor = vec4(result, 1.0);
        }
    )";

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
            FragColor = vec4(1.0, 1.0, 1.0, 1.0); // Couleur blanche
        }
    )";
    // ----------------------------------------------

    // --- Shaders pour les lignes 3D ---
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
    // ----------------------------------------------


    Renderer::Renderer() {
        glEnable(GL_DEPTH_TEST);
        InitShaders();
        InitCrosshair();
        InitLineShader(); // <-- NOUVEL APPEL
        std::cout << "Renderer initialized" << std::endl;
    }

    Renderer::~Renderer() {
        glDeleteProgram(m_ShaderProgram);
        glDeleteProgram(m_CrosshairShaderProgram);
        glDeleteVertexArrays(1, &m_CrosshairVAO);
        glDeleteBuffers(1, &m_CrosshairVBO);

        // --- NOUVEAU: Nettoyage ---
        glDeleteProgram(m_LineShaderProgram);
        glDeleteVertexArrays(1, &m_LineVAO);
        glDeleteBuffers(1, &m_LineVBO);
    }

    void Renderer::InitShaders() {
        // Vertex Shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        // Fragment Shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        // Shader Program
        m_ShaderProgram = glCreateProgram();
        glAttachShader(m_ShaderProgram, vertexShader);
        glAttachShader(m_ShaderProgram, fragmentShader);
        glLinkProgram(m_ShaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    // --- Initialisation du Crosshair ---
    void Renderer::InitCrosshair() {
        // Compile Shaders
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &crosshairVertexShaderSource, NULL);
        glCompileShader(vertexShader);
        // (omission des checks d'erreur par souci de clarté)

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &crosshairFragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        // (omission des checks d'erreur)

        m_CrosshairShaderProgram = glCreateProgram();
        glAttachShader(m_CrosshairShaderProgram, vertexShader);
        glAttachShader(m_CrosshairShaderProgram, fragmentShader);
        glLinkProgram(m_CrosshairShaderProgram);
        // (omission des checks d'erreur)
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // Définir la géométrie (deux lignes, 4 points)
        float size = 10.0f; // 10 pixels
        std::vector<float> vertices = {
            -size, 0.0f,  // Ligne horizontale
             size, 0.0f,
             0.0f, -size, // Ligne verticale
             0.0f,  size
        };

        glGenVertexArrays(1, &m_CrosshairVAO);
        glGenBuffers(1, &m_CrosshairVBO);
        glBindVertexArray(m_CrosshairVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_CrosshairVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void Renderer::InitLineShader() {
        // Compile Shaders
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &lineVertexShaderSource, NULL);
        glCompileShader(vertexShader);
        // (Ajouter des checks d'erreur ici est une bonne idée)

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &lineFragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        // (Checks d'erreur...)

        m_LineShaderProgram = glCreateProgram();
        glAttachShader(m_LineShaderProgram, vertexShader);
        glAttachShader(m_LineShaderProgram, fragmentShader);
        glLinkProgram(m_LineShaderProgram);
        // (Checks d'erreur...)

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // Créer les buffers pour la ligne
        glGenVertexArrays(1, &m_LineVAO);
        glGenBuffers(1, &m_LineVBO);

        // On ne met pas de données ici, on le fera dans DrawLine3D
        // car la ligne change à chaque image
    }

    void Renderer::Clear() {
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::DrawMesh(const Mesh& mesh, const Camera& camera, const glm::mat4& model) {
        glUseProgram(m_ShaderProgram);

        glm::mat4 vp = camera.GetViewProjectionMatrix();
        GLuint vpLoc = glGetUniformLocation(m_ShaderProgram, "u_ViewProjection");
        glUniformMatrix4fv(vpLoc, 1, GL_FALSE, glm::value_ptr(vp));

        GLuint modelLoc = glGetUniformLocation(m_ShaderProgram, "u_Model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        GLuint viewPosLoc = glGetUniformLocation(m_ShaderProgram, "u_ViewPos");
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(camera.GetPosition()));

        mesh.Bind();
        mesh.Draw();
        mesh.Unbind();
    }

    void Renderer::DrawCrosshair(int windowWidth, int windowHeight) {
        // On désactive le test de profondeur pour dessiner par-dessus
        glDisable(GL_DEPTH_TEST);
        glUseProgram(m_CrosshairShaderProgram);

        // Matrice de projection 2D (orthographique)
        glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight);

        // Matrice Model pour centrer le crosshair
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(windowWidth / 2.0f, windowHeight / 2.0f, 0.0f));

        // Envoi des matrices au shader 2D
        GLuint projLoc = glGetUniformLocation(m_CrosshairShaderProgram, "u_Projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        GLuint modelLoc = glGetUniformLocation(m_CrosshairShaderProgram, "u_Model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Dessin
        glLineWidth(2.0f);
        glBindVertexArray(m_CrosshairVAO);
        glDrawArrays(GL_LINES, 0, 4);
        glBindVertexArray(0);

        // On réactive le test de profondeur pour le prochain rendu 3D
        glEnable(GL_DEPTH_TEST);
    }

    void Renderer::DrawLine3D(const glm::vec3& start, const glm::vec3& end, const Camera& camera, const glm::vec3& color) {

        // On désactive le test de profondeur pour que la ligne soit visible
        glDisable(GL_DEPTH_TEST);
        glUseProgram(m_LineShaderProgram);

        // Envoyer les matrices
        glm::mat4 vp = camera.GetViewProjectionMatrix();
        GLuint vpLoc = glGetUniformLocation(m_LineShaderProgram, "u_ViewProjection");
        glUniformMatrix4fv(vpLoc, 1, GL_FALSE, glm::value_ptr(vp));

        // Envoyer la couleur
        GLuint colorLoc = glGetUniformLocation(m_LineShaderProgram, "u_Color");
        glUniform3fv(colorLoc, 1, glm::value_ptr(color));

        // Créer les vertices pour cette ligne
        float vertices[] = {
            start.x, start.y, start.z,
            end.x, end.y, end.z
        };

        // Envoyer les données au GPU
        glBindVertexArray(m_LineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_LineVBO);
        // GL_DYNAMIC_DRAW car les données changent à chaque image
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

        // Définir le layout (juste une position vec3)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Dessiner
        glLineWidth(3.0f); // Ligne un peu épaisse
        glDrawArrays(GL_LINES, 0, 2);

        // Nettoyer
        glBindVertexArray(0);

        // Réactiver le test de profondeur pour le reste de la scène
        glEnable(GL_DEPTH_TEST);
    }
}