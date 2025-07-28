#pragma once

#include <GL/glext.h>
#include <glad/glad.h>

class GLSettingsController {
public:
    static GLSettingsController& getInstance() {
        static GLSettingsController instance;
        return instance;
    }

    GLSettingsController() = default;
    ~GLSettingsController() = default;

    GLSettingsController(const GLSettingsController&) = delete;
    GLSettingsController& operator=(const GLSettingsController&) = delete;
    GLSettingsController(GLSettingsController&&) = delete;
    GLSettingsController& operator=(GLSettingsController&&) = delete;

    void initializeOpenGLSettings() {
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
        glDepthFunc(GL_LESS);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_MULTISAMPLE);

        GLfloat largest_supported_anisotropy;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_supported_anisotropy);
    }

    void disableCullDepth() {
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
    }

    void enableCullDepth() {
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
    }
};