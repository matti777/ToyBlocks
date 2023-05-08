#ifndef TOYBLOCKSCONTROLLER_H
#define TOYBLOCKSCONTROLLER_H

#include <btBulletDynamicsCommon.h>
#include <sys/time.h>

#include "GLController.h"
#include "FpsMeter.h"
#include "Camera.h"
#include "PickingColor.h"
#include "Rect.h"

class Skybox;
class Ground;
class ToyBlock;

// Symbolic name for the "About" texture
static const char* AboutTextureName = "about-texture";

// Dimensions of the "About" texture
static const int AboutTextureWidth = 512;
static const int AboutTextureHeight = 256;

/** Block setups */
enum BlockSetup {
    Simple2DPyramid,
    Simple3DPyramid,
    The5Towers,
    Shootout3Towers,
    TowerArray,

    NumBlockSetups // Number of setups, must be last in enum
};

/** Software state */
enum State {
    StateNormal,
    StateApproachAnimation,
    StateAbout
};

/**
 * This class contains all the control logic & OpenGL code for ToyBlocks.
 * Implemented in portable C++.
 */
class ToyBlocksController : public GLController
{
public:
    ToyBlocksController();
    virtual ~ToyBlocksController();

protected:
    /**
     * Pointer dragging started.
     * @param x X coordinate where drag started
     * @param y Y coordinate where drag started
     */
    virtual void PointerDragStarted(int x, int y);

    /**
     * The pointer was dragged (moved) to new screen coordinates.
     * @param y new Y coordinate
     * @param x new X coordinate
     */
    virtual void PointerDragged(int x, int y);

    /**
     * The pointer dragging has ended.
     */
    virtual void PointerDragEnded();

    /** The screen was tapped (or clicked) at a given location. */
    virtual void TapGesture(int x, int y);

    /** The screen was pinched (gesture). */
    virtual void PinchGesture(float scaleFactor, float rotationAngle);

    /** Does some initialization and calls superclass. */
    virtual void ViewportResized(int width, int height);

    /** Initializes the controller. */
    bool InitController();

    /** Draws everything. */
    virtual void Draw();

    /**
     * Draws the scene with the "picking" shaders and picks a color at given
     * coordinates. Returns the picked color.
     */
    void PickColor(int x, int y, int* red, int* green, int* blue);

    /**
     * Renders the depth map.
     */
    void RenderDepthMap();

    /** Initializes the clickable 'buttons' */
    bool LoadButtonsTextures();

    /** Advance the physics simulation */
    void StepPhysics();

    /** Signals the physics thread to start calculating */
    virtual void SignalPhysicsThread() = 0;

    /** Waits for the physics thread to finish calculating */
    virtual void WaitForPhysicsThread() = 0;

private:
    // Projection matrix setups
    void SetupPerspectiveProjection();
    void SetupOrthoProjection();

    /** Initializes the physics engine */
    void InitPhysics();
    btQuaternion CreateRandomRotation() const;
    bool CreateToyBlock(float x, float y, float z);

    /** Updates the uniforms for the default shading program */
    void UpdateDefaultUniforms(float* modelTransform);

    /** Updates/recalculates the light matrix. */
    void UpdateLightMatrix();

    /** Updates/recalculates the camera matrix. */
    void UpdateCameraMatrix();

    /** Copies the physics transforms to allow for multithreading */
    void CopyPhysicsTransforms();

    /** Gets next unique picking color. Returns false if out of colors. */
    bool NextPickingColor(PickingColor& color);

    /** Picks a block body. */
    void PickBlockBody(int x, int y);

    void DeleteBlocks();
    void InitBlockSetup();

    // Block setups
    void SetupSimpleTower(int numBlocks, int x = 0, int z = 0);
    void SetupSimple2DPyramid();
    void SetupSimple3DPyramid();
    void SetupThe5Towers();
    void SetupShootout3Towers();
    void SetupTowerArray();

    bool CheckForButtonTap(int x, int y);
    void StepAnimation();

protected:
    // Current block setup
    int m_currentBlockSetup;

    // Current state; controls how the UI responds
    State m_state;

    // 2D rects & textures for clickable 'buttons'
    Rect m_nextSetupButtonRect;
    GLuint m_nextSetupButtonTexture;
    Rect m_aboutButtonRect;
    GLuint m_aboutButtonTexture;
    Rect m_aboutImageRect;
    GLuint m_aboutImageTexture;

    // Shader programs
    GLuint m_shadowMapShaderProgram;
    GLuint m_defaultShaderProgram;
    GLuint m_noLightingShaderProgram;
    GLuint m_pickingShaderProgram;

    // Shader uniform locations
    GLuint m_shadowMapShaderMvpLoc;
    GLuint m_defaultShaderMvpLoc;
    GLuint m_defaultShaderModelLoc;
    GLuint m_defaultShaderShadowProjLoc;
    GLuint m_defaultShaderNormalLoc;
    GLuint m_defaultShaderLightLoc;
    GLuint m_defaultShaderShadowMapLoc;
    GLuint m_defaultShaderTextureLoc;
    GLuint m_noLightShaderMvpLoc;
    GLuint m_noLightShaderTextureLoc;
    GLuint m_pickingShaderMvpLoc;
    GLuint m_pickingShaderColorLoc;

    // Shadow mapping
    GLuint m_shadowMapTexture;
    GLuint m_shadowMapFBO;
    GLuint m_shadowMapDepthRenderBuffer;

    // FPS counter
    FpsMeter m_fpsMeter;

    // Approach animation properties
    float m_animationFinalDistance;
    float m_animationDistanceStep;
    float m_animationFinalYRotation;
    float m_animationYRotationStep;
    float m_animationFinalHeight;
    float m_animationHeightStep;

    // the world's objects
    Skybox* m_skybox;
    Ground* m_ground;
    ToyBlock* m_block;
    ToyBlock* m_blockAlt;

    // Block textures
    GLuint m_blocksTexture;

    // Projection matrices
    float m_perspectiveProjectionMatrix[16];
    float m_orthoProjectionMatrix[16];

    // Camera
    Camera m_camera;

    // Whether camera movement should be updated. This will be disabled
    // while a multi-touch event is going on.
    bool m_updateCamera;

    // Previous position of the current pointer drag
    int m_lastDragX;
    int m_lastDragY;

    // Position of the drag start
    int m_dragStartX;
    int m_dragStartY;

    // camera's position's rotation around the y-axis (in degrees)
    float m_yRotation;

    // camera's position's rotation around the x-axis (in degrees)
    float m_xRotation;

    // camera's distance from the look-at target
    float m_cameraDistance;

    // Camera's height from the ground
    float m_cameraHeight;

    // Point light sources rotation around the Y-axis (in degrees)
    float m_lightRotation;

    // Light's camera matrix
    Camera m_light;

    // Light's world position
    float m_lightWorldPosition[3];

    // Next picking color index
    int m_nextPickingColor;

    // Previously picked block body
    btRigidBody* m_pickedBody;

    // Time of the previous physics step / frame render
    timeval m_lastStepTime;

    // Physics engine objects
    btBroadphaseInterface* m_broadphase;
    btDefaultCollisionConfiguration* m_collisionConfiguration;
    btCollisionDispatcher* m_dispatcher;

    btSequentialImpulseConstraintSolver* m_solver;
    btDiscreteDynamicsWorld* m_dynamicsWorld;

    // Physics engine shapes
//    std::vector<btStaticPlaneShape*> m_planeShapes;
//    btStaticPlaneShape* m_groundShape;
//    btRigidBody* m_groundRigidBody;
    std::vector<btRigidBody*> m_groundRigidBodies;

    btCollisionShape* m_blockShape;
    std::vector<btRigidBody*> m_blockRigidBodies;
};

#endif // TOYBLOCKSCONTROLLER_H
