#ifndef GLWIDGET_H
#define GLWIDGET_H

// include OpenGL API
#include "OpenGLAPI.h"

#include <QGLWidget>
#include <QKeyEvent>
#include <QTime>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include "ToyBlocksController.h"

class GLWidget;

#ifdef __BUILD_MULTITHREADED__
/**
 * Physics thread. Separate class because we cant inherit both QWidget
 * and QThread.
 */
class PhysicsThread : public QThread
{
public:
    PhysicsThread(GLWidget* glWidget);
    ~PhysicsThread();

public:
    void run();

private:
    GLWidget* m_glWidget;
};
#endif

/**
 * This class inherits from QGLWidget to provide the Qt platform support for
 * imaging etc system specific APIs. It also inherits from ToyBlocksController
 * to include all the non-platform specific functionality.
 *
 * This class is Qt specific code.
 */
class GLWidget : public QGLWidget, public ToyBlocksController
{
public:
    GLWidget(QWidget* parent);
    virtual ~GLWidget();

protected:
    /** Request a redraw. */
    virtual void Redraw();

    /** Pauses or resumes the rendering timer. */
    virtual void SetPaused(bool paused);

#ifdef __BUILD_MULTITHREADED__
    /** PhysicsThread loop. */
    void PhysicsThreadLoop();
#endif

    /** Signals the physics thread to start calculating */
    virtual void SignalPhysicsThread();

    /** Waits for the physics thread to finish calculating */
    virtual void WaitForPhysicsThread();

    bool event(QEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

    void Debug(const char* fmt, ...) const;
//    void PaintLetterTexture(char letter, QColor color, const QImage& woodImage,
//                            QImage& atlas, int x, int y);
    bool LoadTexture(const char* imageName, GLuint* texture,
                     bool clamp = false, bool useMipmaps = false);

    // From QGLWidget
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

    bool InitializeTextRendering();

private:
    QImage CreateAboutTexture();
//    QImage CreateToyBlockTextureAtlas(QImage& woodImage);

private:
    // Timer for drawing
    QTimer* m_timer;

    // Time of the previous left mouse button press - used for detecting
    // "tap" clicks
    QTime m_lastLeftMouseDown;
    int m_lastLeftMouseX;
    int m_lastLeftMouseY;
#ifdef __BUILD_DESKTOP__
    int m_lastRightMouseX;
    int m_lastRightMouseY;
#endif

#ifdef __BUILD_MULTITHREADED__
    // The physics thread
    PhysicsThread* m_physicsThread;
    bool m_physicsThreadAlive;
    bool m_physicsThreadBusy;
    bool m_physicsThreadWaiting;
    bool m_mainThreadWaiting;
    QMutex m_mutex;
    QWaitCondition m_physicsThreadWait;
    QWaitCondition m_mainThreadWait;

    // Give physics thread access to protected methods
    friend class PhysicsThread;
#endif
};

#endif // GLWIDGET_H
