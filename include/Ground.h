#ifndef GROUND_H
#define GROUND_H

#include <btBulletDynamicsCommon.h>
#include "OpenGLAPI.h"
#include "GLController.h"

// put the ground at -2 meters
static const GLfloat GroundY = -2.0;

/**
 * Represents the ground on which the blocks stand.
 */
class Ground
{
public:
    virtual ~Ground();

    /** Constructs this object */
    static Ground* create(GLController& controller);

    /** Renders this object */
    void Render(GLuint textureUniformLoc);

    /** Creates the collision shapes for ground floor + fence */
    static void CreateShapes(std::vector<btRigidBody*>& rigidBodies);

private:
    Ground(GLController& controller, GLuint floorVertexBuffer,
           GLuint floorIndexBuffer, GLuint fenceVertexBuffer,
           GLuint texture, GLuint fenceTexture);

    static void CreateShape(std::vector<btRigidBody*>& rigidBodies,
                            btVector3 planeNormal, btVector3 position);

private:
    // Reference to the controller
    GLController& m_controller;

    // vertex/index buffers
    GLuint m_floorVertexBuffer;
    GLuint m_floorIndexBuffer;
    GLuint m_fenceVertexBuffer;

    // textures
    GLuint m_floorTexture;
    GLuint m_fenceTexture;
};

#endif // GROUND_H
