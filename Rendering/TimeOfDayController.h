#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "SkyLightInfo.h"

class TimeOfDayController {
private:
    float timeOfDay;
    float dayLength;

    glm::vec3 sunDirection;
    glm::vec3 moonDirection;

    const glm::vec3 nightColor   = glm::vec3(0.01f, 0.01f, 0.03f);
    const glm::vec3 morningColor = glm::vec3(1.0f, 0.6f, 0.7f);
    const glm::vec3 dayColor     = glm::vec3(0.4f, 0.75f, 1.0f);
    const glm::vec3 eveningColor = glm::vec3(1.0f, 0.3f, 0.1f);

    glm::vec3 calculateSkyColor() const {
        float t = timeOfDay;
        float normalizedT = (t + 1.0f) / 2.0f;

        if (normalizedT < 0.2f) {
            return nightColor;
        } else if (normalizedT < 0.3f) {
            float localT = (normalizedT - 0.2f) / 0.1f;
            localT = glm::smoothstep(0.0f, 1.0f, localT);
            return glm::mix(nightColor, morningColor, localT);
        } else if (normalizedT < 0.7f) {
            float localT = (normalizedT - 0.3f) / 0.4f;
            localT = glm::smoothstep(0.0f, 1.0f, localT);
            return glm::mix(morningColor, dayColor, localT);
        } else if (normalizedT < 0.8f) {
            float localT = (normalizedT - 0.7f) / 0.1f;
            localT = glm::smoothstep(0.0f, 1.0f, localT);
            return glm::mix(dayColor, eveningColor, localT);
        } else if (normalizedT <= 1.0f) {
            float localT = (normalizedT - 0.8f) / 0.2f;
            localT = glm::smoothstep(0.0f, 1.0f, localT);
            return glm::mix(eveningColor, nightColor, localT);
        }

        return dayColor;
    }

public:
    TimeOfDayController()
        : timeOfDay(-0.4f), dayLength(60.0f * 20)
    {}

    void update(float deltaTime) {
        float speed = 2.0f / dayLength;
        timeOfDay += speed * deltaTime;

        if (timeOfDay > 1.0f)
            timeOfDay = -1.0f;
        else if (timeOfDay < -1.0f)
            timeOfDay = 1.0f;

        updateSunAndMoonDirections();
    }

    float getTimeOfDay() const {
        return timeOfDay;
    }

    SkyLightInfo getSkyLightInfo() const {
        SkyLightInfo info;
        info.skyColor = calculateSkyColor();
        info.lightDirection = sunDirection;
        if (isDaytime()) {
            info.lightColor = sunLightColor;
            info.lightIntensity = sunLightIntensity;
        } else {
            info.lightColor = moonLightColor;
            info.lightIntensity = moonLightIntensity;
        }
        return info;
    }

private:
    const glm::vec3 sunLightColor = glm::vec3(1.0f, 0.95f, 0.8f);
    const glm::vec3 moonLightColor = glm::vec3(0.6f, 0.65f, 0.8f);

    const float sunLightIntensity = 0.8f;
    const float moonLightIntensity = 0.2f;

    bool isDaytime() const {
        return timeOfDay >= -0.5f && timeOfDay <= 0.5f;
    }

    void updateSunAndMoonDirections() {
        float radius = 1.0f;
        float phase = timeOfDay + 0.5f;
        if (phase < 0.0f) {
            phase += 1.0f;
        } else if (phase > 1.0f) {
            phase -= 1.0f;
        }

        float angle = -glm::pi<float>() + phase * glm::pi<float>();

        sunDirection = glm::normalize(glm::vec3(
            glm::cos(angle) * radius,
            glm::sin(angle),
            glm::sin(angle)
        ));
    }
};
