#include "Skybox.h"

Skybox::Skybox(GLController& controller, GLuint vertexBuffer, GLuint sidesTexture,
               GLuint topTexture)
    : m_controller(controller),
      m_vertexBuffer(vertexBuffer),
      m_sidesTexture(sidesTexture),
      m_topTexture(topTexture)
{
}

Skybox::~Skybox()
{
    // release all OpenGL resources
    glDeleteBuffers(1, &m_vertexBuffer);
    glDeleteTextures(1, &m_sidesTexture);
    glDeleteTextures(1, &m_topTexture);
}

// Skybox textures
static const char* SkyboxSidesTextureName = "skybox-sides.jpg";
static const char* SkyboxTopTextureName = "skybox-top.jpg";

// Skybox side is 2*SkyboxSize
static const float SkyboxSize = 90.0;

const VertexAttribs SkyboxVertices[] = {
    // ("top")
    {-SkyboxSize, SkyboxSize, -SkyboxSize,  0.0, 1.0,    0.0, 1.0, 0.0},
    {-SkyboxSize, SkyboxSize, SkyboxSize,   0.0, 0.0,    0.0, 1.0, 0.0},
    {SkyboxSize, SkyboxSize, -SkyboxSize,   1.0, 1.0,    0.0, 1.0, 0.0},
    {SkyboxSize, SkyboxSize, -SkyboxSize,   1.0, 1.0,    0.0, 1.0, 0.0},
    {-SkyboxSize, SkyboxSize, SkyboxSize,   0.0, 0.0,    0.0, 1.0, 0.0},
    {SkyboxSize, SkyboxSize, SkyboxSize,    1.0, 0.0,    0.0, 1.0, 0.0},

    // ("left")
    {-SkyboxSize, SkyboxSize, -SkyboxSize,  0.0, 1.0,    -1.0, 0.0, 0.0},
    {-SkyboxSize, -SkyboxSize, -SkyboxSize, 0.0, 0.0,    -1.0, 0.0, 0.0},
    {-SkyboxSize, SkyboxSize, SkyboxSize,   0.25, 1.0,   -1.0, 0.0, 0.0},
    {-SkyboxSize, SkyboxSize, SkyboxSize,   0.25, 1.0,   -1.0, 0.0, 0.0},
    {-SkyboxSize, -SkyboxSize, -SkyboxSize, 0.0, 0.0,    -1.0, 0.0, 0.0},
    {-SkyboxSize, -SkyboxSize, SkyboxSize,  0.25, 0.0,   -1.0, 0.0, 0.0},
    // ("back")
    {SkyboxSize, SkyboxSize, -SkyboxSize,   0.75, 1.0,    0.0, 0.0, -1.0},
    {SkyboxSize, -SkyboxSize, -SkyboxSize,  0.75, 0.0,    0.0, 0.0, -1.0},
    {-SkyboxSize, SkyboxSize, -SkyboxSize,  1.0, 1.0,     0.0, 0.0, -1.0},
    {-SkyboxSize, SkyboxSize, -SkyboxSize,  1.0, 1.0,     0.0, 0.0, -1.0},
    {SkyboxSize, -SkyboxSize, -SkyboxSize,  0.75, 0.0,    0.0, 0.0, -1.0},
    {-SkyboxSize, -SkyboxSize, -SkyboxSize, 1.0, 0.0,     0.0, 0.0, -1.0},
    // ("right")
    {SkyboxSize, SkyboxSize, SkyboxSize,    0.5, 1.0,     1.0, 0.0, 0.0},
    {SkyboxSize, -SkyboxSize, SkyboxSize,   0.5, 0.0,     1.0, 0.0, 0.0},
    {SkyboxSize, SkyboxSize, -SkyboxSize,   0.75, 1.0,    1.0, 0.0, 0.0},
    {SkyboxSize, SkyboxSize, -SkyboxSize,   0.75, 1.0,    1.0, 0.0, 0.0},
    {SkyboxSize, -SkyboxSize, SkyboxSize,   0.5, 0.0,     1.0, 0.0, 0.0},
    {SkyboxSize, -SkyboxSize, -SkyboxSize,  0.75, 0.0,    1.0, 0.0, 0.0},
    // ("front")
    {-SkyboxSize, SkyboxSize, SkyboxSize,   0.25, 1.0,    0.0, 0.0, 1.0},
    {-SkyboxSize, -SkyboxSize, SkyboxSize,  0.25, 0.0,    0.0, 0.0, 1.0},
    {SkyboxSize, SkyboxSize, SkyboxSize,    0.5, 1.0,     0.0, 0.0, 1.0},
    {SkyboxSize, SkyboxSize, SkyboxSize,    0.5, 1.0,     0.0, 0.0, 1.0},
    {-SkyboxSize, -SkyboxSize, SkyboxSize,  0.25, 0.0,    0.0, 0.0, 1.0},
    {SkyboxSize, -SkyboxSize, SkyboxSize,   0.5, 0.0,     0.0, 0.0, 1.0},
//    // side 3 ("bottom") - 'D'
//    {-SkyboxSize, -SkyboxSize, SkyboxSize,  0.75, 1.0,    0.0, -1.0, 0.0},
//    {-SkyboxSize, -SkyboxSize, -SkyboxSize, 0.75, 0.5,    0.0, -1.0, 0.0},
//    {SkyboxSize, -SkyboxSize, SkyboxSize,   1.0, 1.0,    0,0,0,0,    0.0, -1.0, 0.0},
//    {SkyboxSize, -SkyboxSize, SkyboxSize,   1.0, 1.0,    0,0,0,0,    0.0, -1.0, 0.0},
//    {-SkyboxSize, -SkyboxSize, -SkyboxSize, 0.75, 0.5,    0,0,0,0,    0.0, -1.0, 0.0},
//    {SkyboxSize, -SkyboxSize, -SkyboxSize,  1.0, 0.5,    0,0,0,0,    0.0, -1.0, 0.0},
};

static const int NumTopVertices = 6; // only one side, 2 triangles
static const int NumSidesVertices = 4 * 6; // 4 sides, 8 triangles

Skybox* Skybox::create(GLController& controller)
{
    GLuint vertexBuffer;

    // create vertex/index buffers
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SkyboxVertices),
                 SkyboxVertices, GL_STATIC_DRAW);

    // load the textures
    GLuint sidesTexture;
    if ( !controller.LoadTexture(SkyboxSidesTextureName, &sidesTexture, true) )
    {
        return NULL;
    }

    GLuint topTexture;
    if ( !controller.LoadTexture(SkyboxTopTextureName, &topTexture, true) )
    {
        return NULL;
    }

    return new Skybox(controller, vertexBuffer, sidesTexture, topTexture);
}

void Skybox::Render(GLuint textureUniformLocation)
{
    // Disable some unnecessery stuff
//    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
//    glDisable(GL_BLEND);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glVertexAttribPointer(COORD_INDEX, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribs),
                          (const GLvoid*)offsetof(VertexAttribs, x));
    glVertexAttribPointer(TEXCOORD_INDEX, 2, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribs),
                          (const GLvoid*)offsetof(VertexAttribs, u));

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(textureUniformLocation, 0);

    // Draw the top of the skybox
    glBindTexture(GL_TEXTURE_2D, m_topTexture);
    glDrawArrays(GL_TRIANGLES, 0, NumTopVertices);

    // Draw the sides of the skybox
    glBindTexture(GL_TEXTURE_2D, m_sidesTexture);
    glDrawArrays(GL_TRIANGLES, NumTopVertices, NumSidesVertices);

    // Restore culling and depth check
//    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
//    glEnable(GL_BLEND);
}
