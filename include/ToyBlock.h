#ifndef TOYBLOCK_H
#define TOYBLOCK_H

#include "OpenGLAPI.h"
#include "GLController.h"

// half of the block's side
static const float ToyBlockSize = 1.0;

// Dimensions of a block
static const float ToyBlockWidth = 2 * ToyBlockSize;
static const float ToyBlockHeight = 2 * ToyBlockSize;

// Name of the texture
static const char* const ToyBlockTextureName = "blocks-texture.jpg";

/** Available block lettering combinations */
enum BlockLettering {
    BlockDefaultLettering,
    BlockAltLettering
};

/**
 * Represents the cube shaped toy block with distinct texture on each side.
 */
class ToyBlock
{
public:
    virtual ~ToyBlock();

    /** Constructs this object */
    static ToyBlock* create(GLController& controller, BlockLettering lettering);

    /** Sets up this object for rendering. */
//    void PrepareRender(GLuint textureUniformLoc);

    /** Renders this object */
    void Render();

private:
    ToyBlock(GLController& controller, GLuint vertexBuffer);

private:
    // Reference to the controller
    GLController& m_controller;

    // vertex/index buffers
    GLuint m_vertexBuffer;

    // textures
//    GLuint m_texture;
};

#endif // TOYBLOCK_H
