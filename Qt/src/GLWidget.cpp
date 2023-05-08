#include <QDebug>
#include <QCoreApplication>
#include <QPinchGesture>
#include <QTimer>
#include <math.h>

#include "GLWidget.h"
#include "ToyBlock.h"

// Threshold value (in ms) for detecting a "click" or a "tap"
static const int ClickThreshold = 200;

#ifdef __BUILD_MULTITHREADED__
PhysicsThread::PhysicsThread(GLWidget* glWidget)
    : m_glWidget(glWidget)
{
}

PhysicsThread::~PhysicsThread()
{
    m_glWidget = NULL;
}

void PhysicsThread::run()
{
    m_glWidget->PhysicsThreadLoop();
    m_glWidget->Debug("PhysicsThread::run() exiting..");
}
#endif

GLWidget::GLWidget(QWidget* parent)
    : QGLWidget(parent),
      m_timer(NULL),
      m_lastLeftMouseX(0),
      m_lastLeftMouseY(0)
#ifdef __BUILD_MULTITHREADED__
    ,
      m_physicsThread(NULL),
      m_physicsThreadAlive(true),
      m_physicsThreadBusy(false),
      m_physicsThreadWaiting(false),
      m_mainThreadWaiting(false)
    #endif
{
    setAutoFillBackground(false);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_StyledBackground,false);
    setAttribute(Qt::WA_AcceptTouchEvents);
    setAutoBufferSwap(false);

    // Grab gestures
    grabGesture(Qt::PinchGesture);

#ifdef __BUILD_MULTITHREADED__
    // Start the physics thread
    Debug("Multi-threading for physics calculations.");
    m_physicsThread = new PhysicsThread(this);
    m_physicsThread->start();
#else
    Debug("Single-threading for physics calculations.");
#endif

    //TODO use this to detect the need for multithreading
//    Debug("numThreads = %d", QThread::currentThread()->idealThreadCount());
}

GLWidget::~GLWidget()
{
    // Stop the timer
    if ( m_timer != NULL )
    {
        m_timer->stop();
        delete m_timer;
    }

#ifdef __BUILD_MULTITHREADED__
    if ( m_physicsThread != NULL )
    {
        // Stop the physics thread
        m_physicsThreadAlive = false;
        SignalPhysicsThread();
        m_physicsThread->wait(100);
        delete m_physicsThread;
    }
#endif
}

void GLWidget::Debug(const char* fmt, ...) const
{
    va_list args;
    va_start(args, fmt);
#ifdef QT_DEBUG
    char msg[1024];
    vsprintf(msg, fmt, args);
    qDebug() << msg;
#endif
    va_end(args);
}

#ifdef __BUILD_MULTITHREADED__
void GLWidget::PhysicsThreadLoop()
{
    while ( m_physicsThreadAlive )
    {
        // Wait for main thread to signal
        m_mutex.lock();
        m_physicsThreadWaiting = true;
        m_physicsThreadWait.wait(&m_mutex);
        m_physicsThreadWaiting = false;
        m_mutex.unlock();

        // Check for exit condition
        if ( !m_physicsThreadAlive )
        {
            return;
        }

        // Advance the physics simulation while main thread renders
        StepPhysics();

        // Signal main thread about completion
        m_mutex.lock();
        m_physicsThreadBusy = false;
        if  ( m_mainThreadWaiting )
        {
            m_mainThreadWait.wakeAll();
        }
        m_mutex.unlock();
    }
}
#endif

void GLWidget::SignalPhysicsThread()
{
#ifdef __BUILD_MULTITHREADED__
    m_mutex.lock();
    if ( m_physicsThreadWaiting )
    {
        m_physicsThreadBusy = true;
        m_physicsThreadWait.wakeAll();
    }
    m_mutex.unlock();
#endif
}

void GLWidget::WaitForPhysicsThread()
{
#ifdef __BUILD_MULTITHREADED__
    m_mutex.lock();
    if ( m_physicsThreadBusy )
    {
        m_mainThreadWaiting = true;
        m_mainThreadWait.wait(&m_mutex);
        m_mainThreadWaiting = false;
    }
    m_mutex.unlock();
#else
    // Single-threaded version: calculate physics now
    StepPhysics();
#endif
}

void GLWidget::SetPaused(bool paused)
{
    if ( paused )
    {
        m_timer->stop();
    }
    else
    {
        m_timer->start();
    }
}

bool GLWidget::InitializeTextRendering()
{
    m_fontTextureHeight = 32;
    int fontBaselineStep = 4;
    QFont font("Fixed", 16);

    QFontMetrics fontMetrics(font);
    QString alphabet(RenderableAlphabet);
    int alphabetWidth = fontMetrics.width(alphabet);
    int imageWidth = alphabetWidth;

    // calculate nearest (higher) power-of-2 width to use as texture width
    for ( int i = 3; i <= 12; i++ )
    {
        if ( pow(2.0f, i) > alphabetWidth )
        {
            imageWidth = pow(2.0f, i);
            break;
        }
    }

    // create the text texture atlas
    QImage image(imageWidth, m_fontTextureHeight, QImage::Format_ARGB32);
    image.fill(Qt::green);
    QPainter painter;
    painter.begin(&image);
    painter.setRenderHints(QPainter::HighQualityAntialiasing |
                           QPainter::TextAntialiasing);
    painter.setFont(font);

    // draw the text twice with 1,1 pixel displacement to create a shadow
    painter.setBrush(Qt::red);
    painter.setPen(Qt::black);
    painter.drawText(1, fontMetrics.ascent() - fontBaselineStep - 1, alphabet);
    painter.setPen(Qt::white);
    painter.drawText(0, fontMetrics.ascent() - fontBaselineStep, alphabet);
    painter.end();

    // calculate widths & texture u's for each char
    m_alphabetInfo = new AlphabetCharInfo[alphabet.length()];
    float prevRightU = 0.0;
    float totalWidth = 0.0;

    for ( int i = 0; i < alphabet.length(); i++ )
    {
        m_alphabetInfo[i].m_width = fontMetrics.width(alphabet[i]);
        m_alphabetInfo[i].m_height = m_fontTextureHeight;
        totalWidth += m_alphabetInfo[i].m_width;
        m_alphabetInfo[i].m_uleft = prevRightU;
        m_alphabetInfo[i].m_uright = totalWidth / (float)imageWidth;
        prevRightU = m_alphabetInfo[i].m_uright;
    }

    // finally copy the texture atlas over to OpenGL
    glGenTextures(1, &m_fontTextureAtlas);
    glBindTexture(GL_TEXTURE_2D, m_fontTextureAtlas);
    image = GLWidget::convertToGLFormat(image);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, m_fontTextureHeight, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    return true;
}

QImage GLWidget::CreateAboutTexture()
{
    QImage image(AboutTextureWidth, AboutTextureHeight, QImage::Format_ARGB32);
    QPainter painter;
    painter.begin(&image);

    // Create transparent background
    QColor bgColor(0,0,0, 200);
    painter.setBrush(bgColor);
    painter.drawRoundedRect(0, 0, AboutTextureWidth, AboutTextureHeight, 20, 20);
#if defined(Q_WS_MAEMO_6)
    QFont font("Serif", 14); // Harmattan
#elif defined(Q_OS_SYMBIAN)
    QFont font("Serif", 6); // Symbian
#else
//    QFont font("Serif", 4); // Android
    QFont font;
#endif
    QFontMetrics fontMetrics(font);
    painter.setFont(font);

    QString header("ToyBlocks 1.1");
    int headerWidth = fontMetrics.width(header);
    painter.setPen(Qt::white);

    painter.drawText((AboutTextureWidth - headerWidth) / 2, 30, header);
    painter.drawLine((AboutTextureWidth - headerWidth) / 2, 37,
                     (AboutTextureWidth + headerWidth) / 2, 37);

    QColor grayColor(150, 150, 150);
    painter.setPen(grayColor);
    painter.drawText(15, 65, "* Rotate the scene by dragging");
    painter.drawText(15, 85, "* Push the blocks around by tapping on them");
    painter.drawText(15, 105, "* Toss the blocks by dragging from them");
    painter.drawText(15, 125, "* Zoom by pinching");
    painter.drawText(15, 145, "* Move light by rotating");
    painter.drawText(15, 165, "* Resume by tapping the screen");

    painter.setPen(Qt::white);
    painter.drawText(15, 190, "Copyright 2011 by Matti Dahlbom (matti@777-team.org)");
    painter.setPen(grayColor);
    painter.drawText(15, 210, "* Physics by Bullet Physics");
    painter.drawText(15, 230, "* Skybox texture by Hipshot");

    painter.end();

    return image;
}

// Thickness of the block face borders
//const int BorderWidth = 15;

//void GLWidget::PaintLetterTexture(char letter, QColor color,
//                                  const QImage& woodImage, QImage& atlas,
//                                  int x, int y)
//{
//    QImage copy = woodImage.copy();
//#ifdef __BUILD_DESKTOP__
//    QFont font("Serif", 220, QFont::Bold);
//#else
//    QFont font("Serif", 180, QFont::Bold);
//#endif
//    QPainter painter;
//    painter.begin(&copy);
//    painter.setRenderHints(QPainter::HighQualityAntialiasing |
//                           QPainter::TextAntialiasing);

//    // draw the letter in the middle
//    //TODO draw outline to the text
//    painter.setFont(font);
//    painter.setPen(color);
//    painter.drawText(45, 205, QString(letter));

//    // draw the borders
//    //TODO draw nice rounded corners inside the corners
//    painter.setBrush(color);
//    painter.drawRect(0, 0, copy.width(), BorderWidth); // top
//    painter.drawRect(0, 0, BorderWidth, copy.width()); // left
//    painter.drawRect(0, copy.height() - BorderWidth,
//                     copy.width(), copy.height()); // bottom
//    painter.drawRect(copy.width() - BorderWidth, 0,
//                     copy.width(), copy.height()); // right
//    painter.end();

//    // draw the created texture on the texture atlas at a given location
//    painter.begin(&atlas);
//    painter.drawImage(x, y, copy);
//    painter.end();
//}

//QImage GLWidget::CreateToyBlockTextureAtlas(QImage& woodImage)
//{
//    // Create the texture atlas to fit copies of the original image as such:
//    // A  B  C  D
//    // E  F  G  H
//    // this will be 1024x512 pixels as wood texture is 256x256

//    int w = woodImage.width();
//    int h = woodImage.height();

//    QImage atlas(w * 4, h * 2, QImage::Format_ARGB32);

//    PaintLetterTexture('A', Qt::blue, woodImage, atlas, 0, 0);
//    PaintLetterTexture('B', Qt::red, woodImage, atlas, w, 0);
//    PaintLetterTexture('C', Qt::yellow, woodImage, atlas, 2 * w, 0);
//    PaintLetterTexture('D', Qt::green, woodImage, atlas, 3 * w, 0);
//    PaintLetterTexture('E', Qt::magenta, woodImage, atlas, 0, h);
//    PaintLetterTexture('F', Qt::white, woodImage, atlas, w, h);
//    PaintLetterTexture('G', Qt::darkYellow, woodImage, atlas, 2 * w, h);
//    PaintLetterTexture('H', Qt::darkMagenta, woodImage, atlas, 3 * w, h);

//    return atlas;
//}

bool GLWidget::LoadTexture(const char* imageName, GLuint* texture,
                           bool clamp, bool useMipmaps)
{
//    Debug("GLWidget::LoadTexture()=%s", imageName);

    QImage image;
    QString imageNameS = QString(imageName);
    if ( imageNameS == AboutTextureName )
    {
        image = CreateAboutTexture();
    }
    else
    {
        QString resImgName = QString(":/") + imageNameS;
        if ( !image.load(resImgName) )
        {
            qDebug() << "failed to load texture " << imageName;
            return false;
        }
    }

    QImage textureMap = convertToGLFormat(image);
    if ( textureMap.isNull() )
    {
        qDebug() << "failed to convert texture to GL format.";
        return false;
    }

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 textureMap.width(), textureMap.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, textureMap.bits());
    if ( useMipmaps )
    {
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_NEAREST);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if ( clamp )
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    return true;
}

void GLWidget::Redraw()
{
    updateGL();
}

void GLWidget::paintGL()
{
    m_fpsMeter.StartFrame();

    // call superclass to draw the world
    Draw();

    // make the drawn stuff visible
    swapBuffers();

    m_fpsMeter.EndFrame();
}

void GLWidget::initializeGL()
{
#ifdef __BUILD_DESKTOP__
    // init GLew extension library
    GLenum err = glewInit();
    if ( err != GLEW_OK )
    {
        qWarning() << "glewInit() failed: '" << glewGetErrorString(err)
                   << "', exiting..";
        QCoreApplication::exit(-1);
        return;
    }
#endif

    // initialize the parent controller
    if ( !InitController() )
    {
        qWarning() << "InitController() failed, exiting..";
        QCoreApplication::quit();
        return;
    }

    // Create a timer and connect it to the rendering method
    m_timer = new QTimer();
    QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    m_timer->start(16);
}

void GLWidget::resizeGL(int width, int height)
{
    // call superclass
    ViewportResized(width, height);
}

bool GLWidget::event(QEvent* event)
{
    if ( event->type() == QEvent::Gesture )
    {
        QGestureEvent* gEvent = static_cast<QGestureEvent*>(event);
        QGesture* gesture = gEvent->gesture(Qt::PinchGesture);
        if ( gesture != NULL )
        {
            QPinchGesture* pinch = static_cast<QPinchGesture*>(gesture);
            PinchGesture(pinch->scaleFactor(), pinch->rotationAngle());
            gEvent->accept();
            return true;
        }
    }
    else if ( (event->type() ==  QEvent::TouchBegin) ||
              (event->type() ==  QEvent::TouchUpdate) ||
              (event->type() ==  QEvent::TouchEnd) )
    {
        // Detect multitouches and block camera updates while they happen
        QTouchEvent* touchEvent = static_cast<QTouchEvent*>(event);
        m_updateCamera = (touchEvent->touchPoints().size() <= 1);
        touchEvent->accept();
        return true;
    }

    return QGLWidget::event(event);
}

void GLWidget::keyPressEvent(QKeyEvent* event)
{
    if ( event->key() == Qt::Key_Escape )
    {
        QCoreApplication::quit();
    }
}

void GLWidget::mousePressEvent(QMouseEvent* event)
{
    if ( event->buttons() & Qt::LeftButton )
    {
        m_lastLeftMouseDown.start();
        m_lastLeftMouseX = event->x();
        m_lastLeftMouseY = event->y();
        PointerDragStarted(event->x(), event->y());
    }

#ifdef __BUILD_DESKTOP__
    if ( event->buttons() & Qt::RightButton )
    {
        m_lastRightMouseX = event->x();
        m_lastRightMouseY = event->y();
    }
#endif
}

void GLWidget::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    if ( m_lastLeftMouseDown.elapsed() < ClickThreshold )
    {
        TapGesture(m_lastLeftMouseX, m_lastLeftMouseY);
    }
    else
    {
        PointerDragEnded();
    }
}

void GLWidget::mouseMoveEvent(QMouseEvent* event)
{
    if ( event->buttons() & Qt::LeftButton )
    {
        PointerDragged(event->x(), event->y());
    }

#ifdef __BUILD_DESKTOP__
    if ( event->buttons() & Qt::RightButton )
    {
        // Simulate pinch & rotate
        int dx = event->x() - m_lastRightMouseX;
        int dy = event->y() - m_lastRightMouseY;
        m_lastRightMouseX = event->x();
        m_lastRightMouseY = event->y();

        if ( dx != 0 )
        {
            // rotate
            PinchGesture(1.0, 1.0 - dx);
        }

        if ( dy != 0 )
        {
            // pinch
            PinchGesture(1.0 + dy * 0.03, 1.0);
        }
    }
#endif
}
