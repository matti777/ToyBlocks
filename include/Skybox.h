#ifndef SKYBOX_H
#define SKYBOX_H

#include "OpenGLAPI.h"
#include "GLController.h"

/**
 * Represents the sky around the objects. This object must be drawn with
 * rotation only, before everything else and with depth check turned off
 * for performance. The box is a cube without a bottom; sides textured with
 * a 4x1 texture atlas and the top with its own texture.
 */
class Skybox
{
public:
    virtual ~Skybox();

    /** Constructs this object */
    static Skybox* create(GLController& controller);

    /** Renders this object */
    void Render(GLuint textureUniformLocation);

private:
    Skybox(GLController& controller, GLuint vertexBuffer, GLuint sidesTexture,
           GLuint topTexture);

private:
    // Reference to the controller
    GLController& m_controller;

    // vertex/index buffers
    GLuint m_vertexBuffer;

    // textures
    GLuint m_sidesTexture;
    GLuint m_topTexture;
};

#endif // SKYBOX_H
