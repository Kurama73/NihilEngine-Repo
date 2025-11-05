#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>

namespace NihilEngine {

struct AtmosphericEffect {
    std::string type; // "fog", "rain", "snow", "dust"
    float intensity;
    glm::vec3 color;
    float density;
};

struct EnvironmentalLight {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    float range;
};

class Environment {
public:
    Environment();

    // Atmosphere
    void setTimeOfDay(float time); // 0.0 = midnight, 0.5 = noon, 1.0 = midnight
    void addAtmosphericEffect(const AtmosphericEffect& effect);
    void updateAtmosphere(float deltaTime);

    // Lighting
    void addLight(const EnvironmentalLight& light);
    void updateLights(float deltaTime);

    // Weather
    void setWeather(const std::string& weatherType, float intensity);
    void updateWeather(float deltaTime);

    // Getters for rendering
    glm::vec3 getSkyColor() const;
    glm::vec3 getAmbientLight() const;
    std::vector<EnvironmentalLight> getLights() const;
    std::vector<AtmosphericEffect> getEffects() const;

private:
    float m_TimeOfDay;
    glm::vec3 m_SkyColor;
    glm::vec3 m_AmbientLight;

    std::vector<EnvironmentalLight> m_Lights;
    std::vector<AtmosphericEffect> m_Effects;

    std::string m_CurrentWeather;
    float m_WeatherIntensity;
};

}