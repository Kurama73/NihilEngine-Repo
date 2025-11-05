#include <NihilEngine/Environment.h>
#include <algorithm>

namespace NihilEngine {

Environment::Environment() : m_TimeOfDay(0.5f), m_WeatherIntensity(0.0f) {
    updateAtmosphere(0.0f);
}

void Environment::setTimeOfDay(float time) {
    m_TimeOfDay = std::clamp(time, 0.0f, 1.0f);
    updateAtmosphere(0.0f);
}

void Environment::addAtmosphericEffect(const AtmosphericEffect& effect) {
    m_Effects.push_back(effect);
}

void Environment::updateAtmosphere(float deltaTime) {
    // Calculate sky color based on time of day
    float sunHeight = sin(m_TimeOfDay * 2.0f * 3.14159f - 3.14159f * 0.5f);

    if (sunHeight > 0.0f) {
        // Daytime
        m_SkyColor = glm::mix(glm::vec3(0.5f, 0.7f, 1.0f), glm::vec3(0.0f, 0.3f, 0.8f), sunHeight);
        m_AmbientLight = glm::vec3(0.6f, 0.6f, 0.6f) * sunHeight;
    } else {
        // Nighttime
        m_SkyColor = glm::vec3(0.0f, 0.0f, 0.1f);
        m_AmbientLight = glm::vec3(0.1f, 0.1f, 0.2f);
    }

    // Add weather effects
    if (m_CurrentWeather == "rain") {
        m_SkyColor = glm::mix(m_SkyColor, glm::vec3(0.3f, 0.3f, 0.4f), m_WeatherIntensity * 0.5f);
    } else if (m_CurrentWeather == "fog") {
        m_SkyColor = glm::mix(m_SkyColor, glm::vec3(0.7f, 0.7f, 0.8f), m_WeatherIntensity);
    }
}

void Environment::addLight(const EnvironmentalLight& light) {
    m_Lights.push_back(light);
}

void Environment::updateLights(float deltaTime) {
    // Update dynamic lights (e.g., flickering torches)
    for (auto& light : m_Lights) {
        // Simple animation
        light.intensity += sin(deltaTime * 2.0f) * 0.1f;
        light.intensity = std::clamp(light.intensity, 0.5f, 1.5f);
    }
}

void Environment::setWeather(const std::string& weatherType, float intensity) {
    m_CurrentWeather = weatherType;
    m_WeatherIntensity = std::clamp(intensity, 0.0f, 1.0f);
    updateAtmosphere(0.0f);
}

void Environment::updateWeather(float deltaTime) {
    // Weather transitions or effects
}

glm::vec3 Environment::getSkyColor() const {
    return m_SkyColor;
}

glm::vec3 Environment::getAmbientLight() const {
    return m_AmbientLight;
}

std::vector<EnvironmentalLight> Environment::getLights() const {
    return m_Lights;
}

std::vector<AtmosphericEffect> Environment::getEffects() const {
    return m_Effects;
}

}