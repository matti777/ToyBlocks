#include <math.h>
#include <stdio.h>

#include "ToyBlocksController.h"
#include "MatrixOperations.h"
#include "ShaderPrograms.h"
#include "Skybox.h"
#include "Ground.h"
#include "ToyBlock.h"
#include "MyMotionState.h"
#include "MyPickingColors.h"

// bias matrix is used to transform unit cube [-1,1] into [0,1]
// all components get c = c*0.5 + 0.5
// Used for shadow mapping.
static float BiasMatrix[16] = {
    0.5, 0.0, 0.0, 0.5,
    0.0, 0.5, 0.0, 0.5,
    0.0, 0.0, 0.5, 0.5,
    0.0, 0.0, 0.0, 1.0 };

// Bias matrix for shadow map rendering; only transform z component
static float ZBiasMatrix[16] = {
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.5,
    0.0, 0.0, 0.0, 1.0 };

// Static identity matrix for avoiding a few MatrixSetIdentity() calls
static float IdentityMatrix[16] = {
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0 };

// Mass of a toy block
static const float ToyBlockInitialMass = 0.9;

// How much the blocks bounce
static const float ToyBlockBounciness = 0.3;

// Friction of the block's surfaces
static const float ToyBlockFriction = 0.85;

// Collision box size (unit?)
static const float ToyBlockCollisionBoxSize = 0.0;

// Force of the push (unit?)
static const float PushForce = 500.0;

// Force multiplier for the toss
static const float TossForceMultiplier = 2.5;

// Displacement value to align the blocks with the ground
static const float BlockDisplaceY = -1.0;

// Space between blocks in most constructs
static const float BlockSpacer = 0.5;
static const float BlockSpacer2 = 0.7;

// Button texture maps
static const char* NextSetupButtonTextureName = "Forward.png";
static const char* AboutButtonTextureName = "Info.png";

// Button constants
static const int ButtonBorderDist = 10;
static const int ButtonSideLen = 64;

// Animation properties
static const int AnimationSteps = 45;
static const float AnimationCameraInitialYRotation = 90;
static const float AnimationCameraInitialDistance = 28;
static const float AnimationCameraInitialHeight = 10;

// Size of the shadow map (ShadowMapSize * ShadowMapSize)
static const int ShadowMapSize = 512;

// Light's rotation around the X axis (defines elevation)
const float LightRotationX = 37.0;

// Light's distance from the origin
const float LightDistance = 25.0;

ToyBlocksController::ToyBlocksController()
    : m_currentBlockSetup(Simple2DPyramid),
      m_state(StateNormal),
      m_nextSetupButtonTexture(0),
      m_defaultShaderProgram(0),
      m_noLightingShaderProgram(0),
      m_pickingShaderProgram(0),
      m_shadowMapTexture(0),
      m_shadowMapFBO(0),
      m_skybox(NULL),
      m_ground(NULL),
      m_block(NULL),
      m_blockAlt(NULL),
      m_blocksTexture(0),
      m_updateCamera(true),
      m_lastDragX(0),
      m_lastDragY(0),
      m_dragStartX(0),
      m_dragStartY(0),
      m_yRotation(0),
      m_xRotation(0),
      m_cameraDistance(12.0),
      m_lightRotation(0),
      m_nextPickingColor(0),
      m_pickedBody(NULL),
      m_broadphase(NULL),
      m_collisionConfiguration(NULL),
      m_dispatcher(NULL),
      m_solver(NULL),
      m_dynamicsWorld(NULL),
      m_blockShape(NULL)
{
    m_lastStepTime.tv_sec = 0;
    m_lastStepTime.tv_usec = 0;
}

ToyBlocksController::~ToyBlocksController()
{
    // release OpenGL resources
    glDeleteProgram(m_defaultShaderProgram);
    glDeleteProgram(m_noLightingShaderProgram);
    glDeleteProgram(m_pickingShaderProgram);

    delete m_skybox;
    delete m_ground;
    delete m_block;
    delete m_blockAlt;

    glDeleteFramebuffers(1, &m_shadowMapFBO);
    glDeleteTextures(1, &m_shadowMapTexture);
    glDeleteRenderbuffers(1, &m_shadowMapDepthRenderBuffer);

    //TODO free my own textures (buttons etc)
    glDeleteTextures(1, &m_blocksTexture);

    //TODO delete all physics engine resources

    DeleteBlocks();
}

bool ToyBlocksController::NextPickingColor(PickingColor& color)
{
    if ( m_nextPickingColor < NumPickingColors )
    {
        color = MyPickingColors[m_nextPickingColor++];
        return true;
    }
    else
    {
        Debug("ToyBlocksController::NextPickingColor(): out of colors!");
        return false;
    }
}

void ToyBlocksController::PointerDragStarted(int x, int y)
{
    if ( m_state != StateNormal )
    {
        return;
    }

    m_lastDragX = x;
    m_lastDragY = y;
    m_dragStartX = x;
    m_dragStartY = y;

    // Try to pick a block object at the location
    PickBlockBody(x, y);
}

void ToyBlocksController::PointerDragged(int x, int y)
{
    if ( m_state != StateNormal )
    {
        return;
    }

    int dx = x - m_lastDragX;
    int dy = y - m_lastDragY;
    m_lastDragX = x;
    m_lastDragY = y;

    if ( m_updateCamera && (m_pickedBody == NULL) )
    {
        m_yRotation += (dx / 3.0);
        m_xRotation += (dy / 4.0);

        if ( m_xRotation < 0 )
        {
            m_xRotation = 0;
        }
        if ( m_xRotation > 45 )
        {
            m_xRotation = 45;
        }

        // Update camera matrix
        UpdateCameraMatrix();

        // Request a redraw
        Redraw();
    }
}

void ToyBlocksController::PointerDragEnded()
{
    // Toss the picked block if any
    if ( m_pickedBody != NULL )
    {
        int dx = m_lastDragX - m_dragStartX;
        int dy = m_lastDragY - m_dragStartY;

        // Scale the dx, dy by the camera 'right' and 'up' vectors respectively
        // and add them together to find out the toss direction
        float right[3];
        float up[3];
        m_camera.ExtractRightVector(right);
        m_camera.ExtractUpVector(up);

        float rightMultiplier = -dx * TossForceMultiplier;
        float upMultiplier = dy * TossForceMultiplier;

        float force[3];
        force[0] = (right[0] * rightMultiplier) + (up[0] * upMultiplier);
        force[1] = (right[1] * rightMultiplier) + (up[1] * upMultiplier);
        force[2] = (right[2] * rightMultiplier) + (up[2] * upMultiplier);

        btVector3 tossVector(force[0], force[1], force[2]);
        m_pickedBody->applyForce(tossVector, btVector3(0,0,0));
    }

    // Clear the picked body so it won't affect next events
    m_pickedBody = NULL;
}

bool ToyBlocksController::CheckForButtonTap(int x, int y)
{
    if ( m_nextSetupButtonRect.IsInside(x, y) )
    {
        // Create the next block setup
        m_currentBlockSetup++;
        if ( m_currentBlockSetup >= NumBlockSetups )
        {
            m_currentBlockSetup = Simple2DPyramid;
        }
        WaitForPhysicsThread();

        InitBlockSetup();
        return true;
    }

    if ( m_state == StateNormal )
    {
        // Only show about dialog in 'normal' state
        if ( m_aboutButtonRect.IsInside(x, y) )
        {
            m_state = StateAbout;
            SetPaused(true);
            Redraw();
            return true;
        }
    }

    return false;
}

void ToyBlocksController::TapGesture(int x, int y)
{
//    Debug("ToyBlocksController::TapEvent(): x = %d, y = %d", x, y);

    if ( m_state == StateAbout )
    {
        // Stop showing the "about" image and resume rendering
        m_state = StateNormal;
        SetPaused(false);
        return;
    }

    // Check if buttons were pressed
    if ( CheckForButtonTap(x, y) )
    {
        return;
    }

    if ( m_state == StateNormal )
    {
        if ( m_pickedBody != NULL )
        {
            // Get the camera 'forward' vector and scale it by push force
            float forward[3];
            m_camera.ExtractForwardVector(forward);
            forward[0] *= PushForce;
            forward[1] *= PushForce;
            forward[2] *= PushForce;

            btVector3 pushVector(forward[0], forward[1], forward[2]);
            m_pickedBody->applyForce(pushVector, btVector3(0,0,0));
        }
    }

    // Clear the picked body so it won't affect future events
    m_pickedBody = NULL;
}

void ToyBlocksController::PickBlockBody(int x, int y)
{
    // Only accept taps on the blocks in 'normal' mode
    int red, green, blue;
    PickColor(x, y, &red, &green, &blue);

    // Find the picked object and push it in the direction of the
    // camera's 'forward' vector
    for ( unsigned int i = 0; i < m_blockRigidBodies.size(); i++ )
    {
        btRigidBody* blockBody = m_blockRigidBodies[i];
        ObjectMotionState* motionState =
                static_cast<ObjectMotionState*>(blockBody->getMotionState());
        if ( motionState->GetPickingColor().Matches(red, green, blue) )
        {
            m_pickedBody = blockBody;
            break;
        }
    }
}

void ToyBlocksController::PinchGesture(float scaleFactor, float rotationAngle)
{
//    Debug("scale=%f, rotation=%f", scaleFactor, rotationAngle);
    m_cameraDistance -= ((scaleFactor - 1.0) * 2.5);
    if ( m_cameraDistance <= 2 )
    {
        m_cameraDistance = 2;
    }
    if ( m_cameraDistance > 20.0 )
    {
        m_cameraDistance = 20.0;
    }

    // rotate the light around with the rotationAngle
    m_lightRotation += ((rotationAngle - 1.0) * 0.045);

    // Update camera+light matrices
    UpdateCameraMatrix();
    UpdateLightMatrix();
}

void ToyBlocksController::DeleteBlocks()
{
    for ( unsigned int i = 0; i < m_blockRigidBodies.size(); i++ )
    {
        btRigidBody* body = m_blockRigidBodies[i];
        m_dynamicsWorld->removeRigidBody(body);
        delete body->getMotionState();
        delete body;
    }
    m_blockRigidBodies.clear();
}

void ToyBlocksController::SetupSimpleTower(int numBlocks, int x, int z)
{
    for ( int i = 0; i < numBlocks; i++ )
    {
        CreateToyBlock(x, i * ToyBlockHeight, z);
    }
//    CreateToyBlock(x, ToyBlockHeight, z);
//    CreateToyBlock(x, 2*ToyBlockHeight, z);
//    CreateToyBlock(x, 3*ToyBlockHeight, z);
//    CreateToyBlock(x, 4*ToyBlockHeight, z);
}

void ToyBlocksController::SetupSimple2DPyramid()
{
    // horizontal distance between block centers
    float dist = BlockSpacer + ToyBlockWidth;
    CreateToyBlock(-dist, 0.0, 0.0);
    CreateToyBlock(0.0, 0.0, 0.0);
    CreateToyBlock(dist, 0.0, 0.0);

    float halfdist = (ToyBlockWidth + BlockSpacer) / 2;
    CreateToyBlock(-halfdist, ToyBlockHeight, 0.0);
    CreateToyBlock(halfdist, ToyBlockHeight, 0.0);

    CreateToyBlock(0.0, 2*ToyBlockHeight, 0.0);
}

void ToyBlocksController::SetupSimple3DPyramid()
{
    float dist = BlockSpacer + ToyBlockWidth;
    CreateToyBlock(-dist, 0.0, dist);
    CreateToyBlock(dist, 0.0, dist);
    CreateToyBlock(0.0, 0.0, 0.0);
    CreateToyBlock(-dist, 0.0, -dist);
    CreateToyBlock(dist, 0.0, -dist);

    float halfdist = (ToyBlockWidth + BlockSpacer) / 2;
    CreateToyBlock(-halfdist, ToyBlockHeight, halfdist);
    CreateToyBlock(halfdist, ToyBlockHeight, halfdist);
    CreateToyBlock(-halfdist, ToyBlockHeight, -halfdist);
    CreateToyBlock(halfdist, ToyBlockHeight, -halfdist);

    CreateToyBlock(0.0, 2*ToyBlockHeight, 0.0);
}

void ToyBlocksController::SetupThe5Towers()
{
    const int Dist = 5;
    SetupSimpleTower(3, -Dist, -Dist);
    SetupSimpleTower(3, Dist, -Dist);
    SetupSimpleTower(3, Dist, Dist);
    SetupSimpleTower(3, -Dist, Dist);
    SetupSimpleTower(5, 0, 0);
}

void ToyBlocksController::SetupShootout3Towers()
{
    const int Dist = 4;
    SetupSimpleTower(5, 0, -Dist);
    SetupSimpleTower(5, -Dist, 0);
    SetupSimpleTower(5, Dist, 0);

    CreateToyBlock(-2, 0, 6);
    CreateToyBlock(2, 0, 6);
}

void ToyBlocksController::SetupTowerArray()
{
    const int Dist = 4;
    for ( int i = -1; i <= 1; i++ )
    {
        for ( int j = -1; j <= 1; j++ )
        {
            SetupSimpleTower(5, i * Dist, j * Dist);
        }
    }
}

void ToyBlocksController::InitBlockSetup()
{
    // Delete existing blocks and their physics engine resources
    DeleteBlocks();

    // Reset picking color index
    m_nextPickingColor = 0;

    // Reset camera, light and transforms to an initial position
    m_animationFinalDistance = 10;
    m_yRotation = 0;
    m_xRotation = 0;
    m_lightRotation = 0;
    UpdateCameraMatrix();
    UpdateLightMatrix();

    switch ( m_currentBlockSetup )
    {
    case Simple2DPyramid:
        m_animationFinalDistance = 7;
        SetupSimple2DPyramid();
        break;
    case Simple3DPyramid:
        m_animationFinalDistance = 8;
        SetupSimple3DPyramid();
        break;
    case The5Towers:
        SetupThe5Towers();
        m_animationFinalDistance = 15;
        break;
    case Shootout3Towers:
        m_animationFinalDistance = 15;
        SetupShootout3Towers();
        break;
    case TowerArray:
        m_animationFinalDistance = 18;
        SetupTowerArray();
        break;
    default:
        SetupSimpleTower(5);
        Debug("Illegal block setup!");
        break;
    }

    // Start approach animation
    m_state = StateApproachAnimation;
    m_animationFinalYRotation = 0;
    m_animationFinalHeight = 0;
    m_cameraDistance = AnimationCameraInitialDistance;
    m_cameraHeight = AnimationCameraInitialHeight;
    m_yRotation = AnimationCameraInitialYRotation;
    float f = 1.0 / AnimationSteps;
    m_animationDistanceStep =
            (m_animationFinalDistance - AnimationCameraInitialDistance) * f;
    m_animationYRotationStep =
            (m_animationFinalYRotation - AnimationCameraInitialYRotation) * f;
    m_animationHeightStep =
            (m_animationFinalHeight - AnimationCameraInitialHeight) * f;

    // Make the initial transforms available
    CopyPhysicsTransforms();
}

btQuaternion ToyBlocksController::CreateRandomRotation() const
{
    // Create a rotation by random
    int axis = rand() % 3;
    int rotation = rand() % 3;

    btVector3 rotationAxis;
    btScalar angleDegrees = 90 + rotation * 90;;

    switch ( axis )
    {
    case 0:
        rotationAxis = btVector3(1, 0, 0);
        break;
    case 1:
        rotationAxis = btVector3(0, 1, 0);
        break;
    case 2:
        rotationAxis = btVector3(0, 0, 1);
        break;
    }

    return btQuaternion(rotationAxis, angleDegrees / 180.0 * M_PI);
}

bool ToyBlocksController::CreateToyBlock(float x, float y, float z)
{
    if ( m_blockShape == NULL )
    {
        // Create the shared shape object to match the dimensions of the
        // rendarable object; ToyBlockSize is half of the side of the block
        m_blockShape = new btBoxShape(btVector3(ToyBlockSize, ToyBlockSize,
                                                ToyBlockSize));
    }

    // Add the displacement value due to ground height
    y += BlockDisplaceY;

    // Choose block instance at random
    ToyBlock* block = m_block;
    if ( (rand() % 2) == 0 )
    {
        block = m_blockAlt;
    }

    // Create the motion state. It will reflect the given initial position
    // and orientation of the object
    btQuaternion initialRotation = CreateRandomRotation();
    ObjectMotionState* motionState =
            new ObjectMotionState(btTransform(initialRotation,
                                          btVector3(x, y, z)), block);

    // Set picking color for this motion state
    PickingColor pickingColor;
    NextPickingColor(pickingColor);
    motionState->SetPickingColor(pickingColor);

    // Calculate inertia
    btScalar mass = ToyBlockInitialMass;
    btVector3 inertia(0, 0, 0);
    m_blockShape->calculateLocalInertia(mass, inertia);

    // Construct the rigid body for this block
    btRigidBody::btRigidBodyConstructionInfo
            blockRigidBodyCI(ToyBlockInitialMass, motionState, m_blockShape,
                             inertia);
    blockRigidBodyCI.m_friction = ToyBlockFriction;
    blockRigidBodyCI.m_restitution = ToyBlockBounciness;
    blockRigidBodyCI.m_linearSleepingThreshold = btScalar(0.0f);
    blockRigidBodyCI.m_angularSleepingThreshold = btScalar(0.0f);
    btRigidBody* blockRigidBody = new btRigidBody(blockRigidBodyCI);

    // Add the created body to the world
    m_dynamicsWorld->addRigidBody(blockRigidBody);

    // ..and to our internal list of bodies
    m_blockRigidBodies.push_back(blockRigidBody);

    return true;
}

void ToyBlocksController::InitPhysics()
{
    // create the engine resources
    m_broadphase = new btDbvtBroadphase();
    m_collisionConfiguration = new btDefaultCollisionConfiguration();
    m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
    m_solver = new btSequentialImpulseConstraintSolver;

    // create the 'world' and apply gravity
    m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase,
                                                  m_solver,
                                                  m_collisionConfiguration);
    m_dynamicsWorld->setGravity(btVector3(0, -9.81, 0));

    // Create the ground static shapes and add them all to the world
    Ground::CreateShapes(m_groundRigidBodies);
    for ( unsigned int i = 0; i < m_groundRigidBodies.size(); i++ )
    {
         m_dynamicsWorld->addRigidBody(m_groundRigidBodies[i]);
    }

    // Create the initial blocks setup
    InitBlockSetup();
}

bool ToyBlocksController::LoadButtonsTextures()
{
    if ( !LoadTexture(NextSetupButtonTextureName, &m_nextSetupButtonTexture) )
    {
        return false;
    }

    if ( !LoadTexture(AboutButtonTextureName, &m_aboutButtonTexture) )
    {
        return false;
    }

    // Also create the "about" image texture
    if ( !LoadTexture(AboutTextureName, &m_aboutImageTexture) )
    {
        return false;
    }

    return true;
}

void ToyBlocksController::SetupOrthoProjection()
{
    MatrixOrthographicProjection(m_orthoProjectionMatrix,
                                 -m_viewportWidth/2, m_viewportWidth/2,
                                 -m_viewportHeight/2, m_viewportHeight/2,
                                 -m_viewportWidth/2, m_viewportWidth/2);
}

void ToyBlocksController::SetupPerspectiveProjection()
{
    MatrixPerspectiveProjection(m_perspectiveProjectionMatrix, 90.0,
                                (float)(m_viewportWidth)/m_viewportHeight,
                                0.5f, 150.0f);
}

void ToyBlocksController::ViewportResized(int width, int height)
{
    // Call superclass
    GLController::ViewportResized(width, height);

    // Setup projection matrices
    SetupPerspectiveProjection();
    SetupOrthoProjection();

    // "next setup" button: upper left corner
    m_nextSetupButtonRect.Set(ButtonBorderDist, ButtonBorderDist,
                              ButtonBorderDist + ButtonSideLen,
                              ButtonBorderDist + ButtonSideLen);

    // "about" button: upper right corner
    m_aboutButtonRect.Set(width - (ButtonBorderDist + ButtonSideLen),
                          ButtonBorderDist,
                          width - ButtonBorderDist,
                          ButtonBorderDist + ButtonSideLen);

    // "About" image rect: in the middle of the screen
    m_aboutImageRect.Set((width - AboutTextureWidth) / 2,
                         (height - AboutTextureHeight) / 2,
                         (width + AboutTextureWidth) / 2,
                         (height + AboutTextureHeight) / 2);
}


bool ToyBlocksController::InitController()
{
    // Initialize the random number generator from system clock
    timeval timev;
    gettimeofday(&timev, NULL);
    srand(timev.tv_usec);

    // start with the superclass defaults
    if ( !GLController::InitController() )
    {
        return false;
    }

    // enable vertex attrib arrays
    glEnableVertexAttribArray(COORD_INDEX);
    glEnableVertexAttribArray(TEXCOORD_INDEX);
    glEnableVertexAttribArray(NORMAL_INDEX);

    // Initialize shadow mapping
    if ( !CreateDepthTextureAndFBO(&m_shadowMapFBO, &m_shadowMapTexture,
                                   &m_shadowMapDepthRenderBuffer,
                                   ShadowMapSize, ShadowMapSize) )
    {
        return false;
    }

    // create the shader programs
    if ( m_hasDepthTextureExtension )
    {
        Debug("Loading depth buffer version of the shadow map shader.");
        if ( !LoadShader(&m_shadowMapShaderProgram, DepthShadowMapVertexShader,
                         DepthShadowMapFragmentShader) )
        {
            return false;
        }
        if ( !LoadShader(&m_defaultShaderProgram, DepthDefaultVertexShader,
                         DepthDefaultFragmentShader) )
        {
            return false;
        }
    }
    else
    {
        if ( !LoadShader(&m_shadowMapShaderProgram, ShadowMapVertexShader,
                         ShadowMapFragmentShader) )
        {
            return false;
        }
        if ( !LoadShader(&m_defaultShaderProgram, DefaultVertexShader,
                         DefaultFragmentShader) )
        {
            return false;
        }
    }
    if ( !LoadShader(&m_noLightingShaderProgram, NoLightVertexShader,
                     NoLightFragmentShader) )
    {
        return false;
    }
    if ( !LoadShader(&m_pickingShaderProgram, PickingVertexShader,
                     PickingFragmentShader) )
    {
        return false;
    }

    // Get & store uniform locations
    m_shadowMapShaderMvpLoc = glGetUniformLocation(m_shadowMapShaderProgram,
                                                   "mvp_matrix");
    m_defaultShaderMvpLoc = glGetUniformLocation(m_defaultShaderProgram,
                                                 "mvp_matrix");
    m_defaultShaderModelLoc = glGetUniformLocation(m_defaultShaderProgram,
                                                "model_matrix");
    m_defaultShaderShadowProjLoc = glGetUniformLocation(m_defaultShaderProgram,
                                                        "shadow_proj_matrix");
    m_defaultShaderNormalLoc = glGetUniformLocation(m_defaultShaderProgram,
                                                    "normal_matrix");
    m_defaultShaderLightLoc = glGetUniformLocation(m_defaultShaderProgram,
                                                   "light_pos");
    m_defaultShaderShadowMapLoc = glGetUniformLocation(m_defaultShaderProgram,
                                                       "shadow_texture");
    m_defaultShaderTextureLoc = glGetUniformLocation(m_defaultShaderProgram,
                                                     "texture");
    m_noLightShaderMvpLoc = glGetUniformLocation(m_noLightingShaderProgram,
                                                 "mvp_matrix");
    m_noLightShaderTextureLoc = glGetUniformLocation(m_noLightingShaderProgram,
                                                     "texture");
    m_pickingShaderMvpLoc = glGetUniformLocation(m_pickingShaderProgram,
                                                 "mvp_matrix");
    m_pickingShaderColorLoc = glGetUniformLocation(m_pickingShaderProgram,
                                                   "pick_color");

    // Initialize the clickable 'buttons'
    if ( !LoadButtonsTextures() )
    {
        Debug("InitButtons() failed");
        return false;
    }

    // Load the blocks texture
    if ( !LoadTexture(ToyBlockTextureName, &m_blocksTexture) )
    {
        return false;
    }

    // create the renderable objects
    m_skybox = Skybox::create(*this);
    m_ground = Ground::create(*this);
    m_block = ToyBlock::create(*this, BlockDefaultLettering);
    m_blockAlt = ToyBlock::create(*this, BlockAltLettering);

    if ( (m_skybox == NULL) || (m_ground == NULL) || (m_block == NULL) ||
         (m_blockAlt == NULL) )
    {
        Debug("Object creation failed!");
        return false;
    }

    // Setup light's projection matrix to match the shadow map dimensions
    MatrixPerspectiveProjection(m_light.GetProjectionMatrix(), 90,
                                (float)(ShadowMapSize)/(float)ShadowMapSize,
                                12.0f, 50.0f);

    // Set up the physics engine
    InitPhysics();

    // Additional settings
    glDisable(GL_DITHER);

    return true;
}

void ToyBlocksController::StepPhysics()
{
    btScalar seconds = 0.0;
    timeval now;
    gettimeofday(&now, NULL);

    if ( m_lastStepTime.tv_sec > 0 )
    {
        // there has been a previous step, calculate time between steps
        seconds = (btScalar)(now.tv_sec - m_lastStepTime.tv_sec);
        seconds += (btScalar)((now.tv_usec - m_lastStepTime.tv_usec) * 0.000001);
    }

    m_lastStepTime.tv_sec = now.tv_sec;
    m_lastStepTime.tv_usec = now.tv_usec;

    m_dynamicsWorld->stepSimulation(seconds, 2);
}

void ToyBlocksController::CopyPhysicsTransforms()
{
    for ( unsigned int i = 0; i < m_blockRigidBodies.size(); i++ )
    {
        btRigidBody* blockBody = m_blockRigidBodies[i];
        ObjectMotionState* motionState =
                static_cast<ObjectMotionState*>(blockBody->getMotionState());
        motionState->UpdateObjectTransform();
    }
}

void ToyBlocksController::RenderDepthMap()
{
    // Adjust viewport to match the shadow map size
    glViewport(0, 0, ShadowMapSize, ShadowMapSize);

    // Start using our offscreen FBO & texture
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFBO);

    // check FBO status
    GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if ( fboStatus != GL_FRAMEBUFFER_COMPLETE )
    {
        Debug("RenderDepthMap(): FBO not complete.");
        return;
    }

    // Depth checking on and clear the buffer
    glEnable(GL_DEPTH_TEST);
    if ( m_hasDepthTextureExtension )
    {
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    else
    {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // Start using the depth-rendering program
    glUseProgram(m_shadowMapShaderProgram);

    // Use the light matrix as "camera" for rendering
    float* inverseLightMatrix = m_light.GetInverseCameraMatrix();

    float mvpMatrix[16];

    // Render all the shadow casters; ie the blocks
    for ( unsigned int i = 0; i < m_blockRigidBodies.size(); i++ )
    {
        btRigidBody* blockBody = m_blockRigidBodies[i];
        ObjectMotionState* motionState =
                static_cast<ObjectMotionState*>(blockBody->getMotionState());
        GLfloat objectTransform[16];

        // Transform the object by the light's inverse camera matrix
        MatrixMultiply(motionState->GetObjectTransform(), inverseLightMatrix,
                       objectTransform);

        // Set up the model-view-projection matrix for the light
        MatrixMultiply(objectTransform, m_light.GetProjectionMatrix(),
                       mvpMatrix);
        if ( !m_hasDepthTextureExtension )
        {
            // The RGBA texture approach requires this
            MatrixMultiply(mvpMatrix, ZBiasMatrix, mvpMatrix);
        }
        glUniformMatrix4fv(m_shadowMapShaderMvpLoc, 1, GL_FALSE, mvpMatrix);

        // Render the block (without textures)
        m_block->Render();
    }

    // Go back to using the default render buffer
    glBindFramebuffer(GL_FRAMEBUFFER, DefaultFramebufferId);

    // Reset the viewport
    glViewport(0, 0, m_viewportWidth, m_viewportHeight);
}

// for debugging purposes only ..
#include <QDebug>
void _PrintMatrix4(GLfloat* matrix)
{
    for ( int i = 0; i < 4; i++ )
    {
        qDebug() << matrix[i*4 + 0] << "\t" << matrix[i*4 + 1] << "\t"
                 << matrix[i*4 + 2] << "\t" << matrix[i*4 + 3];
    }
}
void _PrintMatrix3(GLfloat* matrix)
{
    for ( int i = 0; i < 3; i++ )
    {
        qDebug() << matrix[i*3 + 0] << "\t" << matrix[i*3 + 1] << "\t"
                 << matrix[i*3 + 2];
    }
}

void ToyBlocksController::UpdateLightMatrix()
{
    float translation[16];
    MatrixCreateTranslation(translation, 0, 0, LightDistance);
    float yRotationMatrix[16];
    MatrixCreateRotation(yRotationMatrix, m_lightRotation * M_PI / 180.0, 0, 1, 0);
    float xRotationMatrix[16];
    MatrixCreateRotation(xRotationMatrix, LightRotationX * M_PI / 180.0, 1, 0, 0);

    MatrixMultiply(translation, IdentityMatrix, m_light.GetCameraMatrix());
    MatrixMultiply(m_light.GetCameraMatrix(), yRotationMatrix,
                   m_light.GetCameraMatrix());
    MatrixMultiply(m_light.GetCameraMatrix(), xRotationMatrix,
                   m_light.GetCameraMatrix());
    m_light.CalculateInverseCameraMatrix();

    // Calculate the light's world position as: -forward * distance
    // (Z inverted due to OpenGLs coordinate system)
    float* inverseLightMatrix = m_light.GetInverseCameraMatrix();
    m_lightWorldPosition[0] = inverseLightMatrix[8] * LightDistance;
    m_lightWorldPosition[1] = inverseLightMatrix[9] * LightDistance;
    m_lightWorldPosition[2] = inverseLightMatrix[10] * LightDistance;
}

void ToyBlocksController::UpdateCameraMatrix()
{
    float translation[16];
    MatrixCreateTranslation(translation, 0, m_cameraHeight, m_cameraDistance);
    float yRotationMatrix[16];
    MatrixCreateRotation(yRotationMatrix, m_yRotation * M_PI / 180.0, 0, 1, 0);
    float xRotationMatrix[16];
    MatrixCreateRotation(xRotationMatrix, m_xRotation * M_PI / 180.0, 1, 0, 0);

    MatrixMultiply(translation, IdentityMatrix, m_camera.GetCameraMatrix());
    MatrixMultiply(m_camera.GetCameraMatrix(), yRotationMatrix,
                   m_camera.GetCameraMatrix());
    MatrixMultiply(m_camera.GetCameraMatrix(), xRotationMatrix,
                   m_camera.GetCameraMatrix());
    m_camera.CalculateInverseCameraMatrix();
}

void ToyBlocksController::StepAnimation()
{
    m_cameraDistance += m_animationDistanceStep;
    m_cameraHeight += m_animationHeightStep;
    m_yRotation += m_animationYRotationStep;

    if ( m_cameraDistance < m_animationFinalDistance )
    {
        // Time to stop the animation; we're at the starting point
        m_state = StateNormal;
    }

    UpdateCameraMatrix();
}

void ToyBlocksController::UpdateDefaultUniforms(float* modelTransform)
{
    // Transform the light position to object space
    float lightObjectSpacePos[3];
    Transform(modelTransform, m_lightWorldPosition, lightObjectSpacePos);
    glUniform3fv(m_defaultShaderLightLoc, 1, lightObjectSpacePos);

    // Copy the modelview-projection matrix over to GLSL
    // mvp = projection * inverse_camera * model_transform
    GLfloat mvpMatrix[16];
    MatrixMultiply(modelTransform, m_camera.GetInverseCameraMatrix(), mvpMatrix);
    MatrixMultiply(mvpMatrix, m_perspectiveProjectionMatrix, mvpMatrix);
    glUniformMatrix4fv(m_defaultShaderMvpLoc, 1, GL_FALSE, mvpMatrix);

    // Set shadow matrix: bias*light_projection*inverse_light
    float shadowMatrix[16];
    MatrixMultiply(modelTransform, m_light.GetInverseCameraMatrix(), shadowMatrix);
    MatrixMultiply(shadowMatrix, m_light.GetProjectionMatrix(), shadowMatrix);
    MatrixMultiply(shadowMatrix, BiasMatrix, shadowMatrix);
    glUniformMatrix4fv(m_defaultShaderShadowProjLoc, 1, GL_FALSE, shadowMatrix);
}

void ToyBlocksController::Draw()
{
    // Clear the screen & depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if ( m_state == StateApproachAnimation )
    {
        StepAnimation();
    }

    // Render the shadow depth map for this frame
    RenderDepthMap();

    // Use the lighted / shadowed program
    glUseProgram(m_defaultShaderProgram);

    // Pass the generated shadow map to the shader
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_shadowMapTexture);
    glUniform1i(m_defaultShaderShadowMapLoc, 1);

    // Only set texture once as it is shared
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_defaultShaderTextureLoc, 0);
    glBindTexture(GL_TEXTURE_2D, m_blocksTexture);

    // Draw the blocks
    for ( unsigned int i = 0; i < m_blockRigidBodies.size(); i++ )
    {
        btRigidBody* blockBody = m_blockRigidBodies[i];
        ObjectMotionState* motionState =
                static_cast<ObjectMotionState*>(blockBody->getMotionState());

        UpdateDefaultUniforms(motionState->GetObjectTransform());

        // Render the block
        ToyBlock* block = reinterpret_cast<ToyBlock*>(motionState->GetData());
        block->Render();
    }

    // No lighting for the Skybox / 2D drawing
    glUseProgram(m_noLightingShaderProgram);

    // Draw skybox; only use rotations (from camera matrix)
    float rotationMatrix[16];
    ExtractRotation(m_camera.GetCameraMatrix(), rotationMatrix);
    GLfloat mvpMatrix[16];
    MatrixMultiply(rotationMatrix, m_perspectiveProjectionMatrix, mvpMatrix);
    glUniformMatrix4fv(m_noLightShaderMvpLoc, 1, GL_FALSE, mvpMatrix);
    m_skybox->Render(m_noLightShaderTextureLoc);

    // Use the lighted / shadowed program
    glUseProgram(m_defaultShaderProgram);

    // Pass the generated shadow map to the shader
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_shadowMapTexture);
    glUniform1i(m_defaultShaderShadowMapLoc, 3);

    // Render the ground. Must be here for the transparent bits of the fence
    // to show background properly
    UpdateDefaultUniforms(IdentityMatrix);
    m_ground->Render(m_defaultShaderTextureLoc);

    // No lighting for the Skybox / 2D drawing
    glUseProgram(m_noLightingShaderProgram);

    // Create mvp matrix for 2D drawing: ortho_proj * identity
    MatrixMultiply(IdentityMatrix, m_orthoProjectionMatrix, mvpMatrix);
    glUniformMatrix4fv(m_noLightShaderMvpLoc, 1, GL_FALSE, mvpMatrix);

    // DEBUG only; render the depth map on screen
//    Rect depthMapRect(50, 0, 350, 300);
//    DrawImage2D(depthMapRect, m_shadowMapTexture, m_noLightShaderTextureLoc);

    // Draw the buttons
    DrawImage2D(m_nextSetupButtonRect, m_nextSetupButtonTexture,
                m_noLightShaderTextureLoc);
    DrawImage2D(m_aboutButtonRect, m_aboutButtonTexture,
                m_noLightShaderTextureLoc);

    if ( m_state == StateAbout )
    {
        // Draw the "about" image
        DrawImage2D(m_aboutImageRect, m_aboutImageTexture,
                    m_noLightShaderTextureLoc);
    }

    // Draw FPS
//    char fps[16];
//    sprintf(fps, "FPS: %.1f", m_fpsMeter.GetFps());
//    Debug(fps);
//    DrawText(15, 90, fps, m_noLightShaderTextureLoc);

    // Wait for the physics thread to finish calculating
    WaitForPhysicsThread();

    // Copy the physics body transformations
    CopyPhysicsTransforms();

    // Signal the physics thread to start calculating for next frame
    SignalPhysicsThread();
}

void ToyBlocksController::PickColor(int x, int y,
                                    int* red, int* green, int* blue)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_pickingShaderProgram);

    float* inverseCameraMatrix = m_camera.GetInverseCameraMatrix();

    // Draw the blocks
    for ( unsigned int i = 0; i < m_blockRigidBodies.size(); i++ )
    {
        btRigidBody* blockBody = m_blockRigidBodies[i];
        ObjectMotionState* motionState =
                static_cast<ObjectMotionState*>(blockBody->getMotionState());
        GLfloat objectTransform[16];

        // Transform the object by the inverse camera matrix
        MatrixMultiply(motionState->GetObjectTransform(), inverseCameraMatrix,
                       objectTransform);

        // write the object's pick color to shader's uniform
        float pickColorFloat[3];
        motionState->GetPickingColor().ToFloats(pickColorFloat);
        glUniform3fv(m_pickingShaderColorLoc, 1, pickColorFloat);

        // Set the GLSL model-view-projection matrix uniform
        GLfloat mvpMatrix[16];
        MatrixMultiply(objectTransform, m_perspectiveProjectionMatrix, mvpMatrix);
        glUniformMatrix4fv(m_pickingShaderMvpLoc, 1, GL_FALSE, mvpMatrix);

        // Render the block without a texture
        m_block->Render();
    }

    // OpenGL handles y coordinated inverted
    int invertedY = m_viewportHeight - y;

    // read the 4-byte RGBA pixel
    GLubyte pixel[4];
    glReadPixels(x, invertedY, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pixel);

    // Copy the picking color into the parameters
    *red = (int)pixel[0];
    *green = (int)pixel[1];
    *blue = (int)pixel[2];
}
