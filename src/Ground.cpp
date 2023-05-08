#include "Ground.h"

Ground::Ground(GLController& controller, GLuint floorVertexBuffer,
               GLuint floorIndexBuffer, GLuint fenceVertexBuffer,
               GLuint texture, GLuint fenceTexture)
    : m_controller(controller),
      m_floorVertexBuffer(floorVertexBuffer),
      m_floorIndexBuffer(floorIndexBuffer),
      m_fenceVertexBuffer(fenceVertexBuffer),
      m_floorTexture(texture),
      m_fenceTexture(fenceTexture)
{
}

Ground::~Ground()
{
    // release all OpenGL resources
    glDeleteBuffers(1, &m_floorVertexBuffer);
    glDeleteBuffers(1, &m_floorIndexBuffer);
    glDeleteBuffers(1, &m_fenceVertexBuffer);
    glDeleteTextures(1, &m_floorTexture);
    glDeleteTextures(1, &m_fenceTexture);
}

static const char* FloorTextureName = "ground-floor.jpg";
static const GLfloat Size = 25.0;
static const GLfloat TexMax = 5.0; // texture tiling, max value for u/v

static const VertexAttribs FloorVertices[] = {
//    {-Size, GroundY, -Size,  0.0, TexMax,    0,0,0,0,  -0.57735, 0.57735, -0.57735},
//    {-Size, GroundY, Size, 0.0, 0.0,         0,0,0,0,  -0.57735, 0.57735, 0.57735},
//    {Size, GroundY, Size,  TexMax, 0.0,      0,0,0,0,  0.57735, 0.57735, 0.57735},
//    {Size, GroundY, -Size,   TexMax, TexMax, 0,0,0,0,  0.57735, 0.57735, -0.57735}
    {-Size, GroundY, -Size,  0.0, TexMax,    0.0, 1.0, 0.0},
    {-Size, GroundY, Size,   0.0, 0.0,       0.0, 1.0, 0.0},
    {Size, GroundY, Size,    TexMax, 0.0,    0.0, 1.0, 0.0},
    {Size, GroundY, -Size,   TexMax, TexMax, 0.0, 1.0, 0.0}
};

static const GLubyte FloorIndices[] = { 0,1,3, 3,1,2 };
static const int NumFloorIndices = sizeof(FloorIndices) / sizeof(GLubyte);

static const char* FenceTextureName = "ground-fence.png";
static const float FenceDistance = 25.0;
static const float FenceHeight = 7.0;
static const float FenceTexMax = 5.0; // texture tiling, max value for u/v

static const VertexAttribs FenceVertices[] = {
    // "Far"
    { -FenceDistance, GroundY, -FenceDistance,             0.0, 0.0,  0.0, 0.0, 1.0 },
    { FenceDistance, GroundY, -FenceDistance,              FenceTexMax, 0.0,  0.0, 0.0, 1.0 },
    { FenceDistance, GroundY+FenceHeight, -FenceDistance,  FenceTexMax, 1.0,  0.0, 0.0, 1.0 },
    { FenceDistance, GroundY+FenceHeight, -FenceDistance,  FenceTexMax, 1.0,  0.0, 0.0, 1.0 },
    { -FenceDistance, GroundY+FenceHeight, -FenceDistance, 0.0, 1.0,  0.0, 0.0, 1.0 },
    { -FenceDistance, GroundY, -FenceDistance,             0.0, 0.0,  0.0, 0.0, 1.0 },

    // "Left"
    { -FenceDistance, GroundY, -FenceDistance,             0.0, 0.0,  0.0, 0.0, 1.0 },
    { -FenceDistance, GroundY, FenceDistance,              FenceTexMax, 0.0,  0.0, 0.0, 1.0 },
    { -FenceDistance, GroundY+FenceHeight, FenceDistance,  FenceTexMax, 1.0,  0.0, 0.0, 1.0 },
    { -FenceDistance, GroundY+FenceHeight, FenceDistance,  FenceTexMax, 1.0,  0.0, 0.0, 1.0 },
    { -FenceDistance, GroundY+FenceHeight, -FenceDistance, 0.0, 1.0,  0.0, 0.0, 1.0 },
    { -FenceDistance, GroundY, -FenceDistance,             0.0, 0.0,  0.0, 0.0, 1.0 },

    // "Near"
    { -FenceDistance, GroundY, FenceDistance,             0.0, 0.0,  0.0, 0.0, 1.0 },
    { FenceDistance, GroundY, FenceDistance,              FenceTexMax, 0.0,  0.0, 0.0, 1.0 },
    { FenceDistance, GroundY+FenceHeight, FenceDistance,  FenceTexMax, 1.0,  0.0, 0.0, 1.0 },
    { FenceDistance, GroundY+FenceHeight, FenceDistance,  FenceTexMax, 1.0,  0.0, 0.0, 1.0 },
    { -FenceDistance, GroundY+FenceHeight, FenceDistance, 0.0, 1.0,  0.0, 0.0, 1.0 },
    { -FenceDistance, GroundY, FenceDistance,             0.0, 0.0,  0.0, 0.0, 1.0 },

    // "Right"
    { FenceDistance, GroundY, -FenceDistance,             0.0, 0.0,  0.0, 0.0, 1.0 },
    { FenceDistance, GroundY, FenceDistance,              FenceTexMax, 0.0,  0.0, 0.0, 1.0 },
    { FenceDistance, GroundY+FenceHeight, FenceDistance,  FenceTexMax, 1.0,  0.0, 0.0, 1.0 },
    { FenceDistance, GroundY+FenceHeight, FenceDistance,  FenceTexMax, 1.0,  0.0, 0.0, 1.0 },
    { FenceDistance, GroundY+FenceHeight, -FenceDistance, 0.0, 1.0,  0.0, 0.0, 1.0 },
    { FenceDistance, GroundY, -FenceDistance,             0.0, 0.0,  0.0, 0.0, 1.0 },
};

static const int NumFenceVertices = sizeof(FenceVertices) / sizeof(VertexAttribs);

Ground* Ground::create(GLController& controller)
{
    GLuint floorVertexBuffer;
    GLuint floorIndexBuffer;
    GLuint fenceVertexBuffer;

    // create vertex/index buffers
    glGenBuffers(1, &floorVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, floorVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(FloorVertices),
                 FloorVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &floorIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(FloorIndices),
                 FloorIndices, GL_STATIC_DRAW);

    glGenBuffers(1, &fenceVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, fenceVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(FenceVertices),
                 FenceVertices, GL_STATIC_DRAW);

    // load the textures
    GLuint floorTexture;
    if ( !controller.LoadTexture(FloorTextureName, &floorTexture, false, true) )
    {
        return NULL;
    }
    GLuint fenceTexture;
    if ( !controller.LoadTexture(FenceTextureName, &fenceTexture, false, true) )
    {
        return NULL;
    }

    return new Ground(controller, floorVertexBuffer, floorIndexBuffer,
                      fenceVertexBuffer, floorTexture, fenceTexture);
}

void Ground::CreateShape(std::vector<btRigidBody*>& rigidBodies,
                         btVector3 planeNormal, btVector3 position)
{
    btStaticPlaneShape* shape = new btStaticPlaneShape(planeNormal, 0);

    // create the motion state
    btDefaultMotionState* groundMotionState =
            new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1),
                                                 position));

    // body construction info: mass = 0, inertia = 0 vector for static shape
    btRigidBody::btRigidBodyConstructionInfo
                 groundRigidBodyCI(0, groundMotionState, shape,
                                   btVector3(0, 0, 0));
    rigidBodies.push_back(new btRigidBody(groundRigidBodyCI));
}

void Ground::CreateShapes(std::vector<btRigidBody*>& rigidBodies)
{
    // Floor
    CreateShape(rigidBodies, btVector3(0, 1, 0), btVector3(0, GroundY, 0));

    // Fence "far"
    CreateShape(rigidBodies, btVector3(0, 0, 1), btVector3(0, 0, -FenceDistance));

    // Fence "left"
    CreateShape(rigidBodies, btVector3(1, 0, 0), btVector3(-FenceDistance, 0, 0));

    // Fence "right"
    CreateShape(rigidBodies, btVector3(-1, 0, 0), btVector3(FenceDistance, 0, 0));

    // Fence "near"
    CreateShape(rigidBodies, btVector3(0, 0, -1), btVector3(0, 0, FenceDistance));
}

void Ground::Render(GLuint textureUniformLoc)
{
    // Draw the floor
    glBindBuffer(GL_ARRAY_BUFFER, m_floorVertexBuffer);
    glVertexAttribPointer(COORD_INDEX, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribs),
                          (const GLvoid*)offsetof(VertexAttribs, x));
    glVertexAttribPointer(TEXCOORD_INDEX, 2, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribs),
                          (const GLvoid*)offsetof(VertexAttribs, u));
    glVertexAttribPointer(NORMAL_INDEX, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribs),
                          (const GLvoid*)offsetof(VertexAttribs, nx));

    if ( textureUniformLoc > 0 )
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_floorTexture);
        glUniform1i(textureUniformLoc, 0);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_floorIndexBuffer);
    glDrawElements(GL_TRIANGLES, NumFloorIndices, GL_UNSIGNED_BYTE, NULL);

    // Draw the fence
    glBindBuffer(GL_ARRAY_BUFFER, m_fenceVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisable(GL_CULL_FACE);

    glVertexAttribPointer(COORD_INDEX, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribs),
                          (const GLvoid*)offsetof(VertexAttribs, x));
    glVertexAttribPointer(TEXCOORD_INDEX, 2, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribs),
                          (const GLvoid*)offsetof(VertexAttribs, u));
    glVertexAttribPointer(NORMAL_INDEX, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribs),
                          (const GLvoid*)offsetof(VertexAttribs, nx));

    if ( textureUniformLoc > 0 )
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_fenceTexture);
        glUniform1i(textureUniformLoc, 0);
    }

    glDrawArrays(GL_TRIANGLES, 0, NumFenceVertices);
    glEnable(GL_CULL_FACE);
}

