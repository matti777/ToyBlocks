# Add files and directories to ship with the application
# by adapting the examples below.
# file1.source = myfile
# dir1.source = mydir
DEPLOYMENTFOLDERS = # file1 dir1

# If your application uses the Qt Mobility libraries, uncomment
# the following lines and add the respective components to the
# MOBILITY variable.
# CONFIG += mobility
# MOBILITY +=

QT += opengl

# include the Bullet Physics engine
include(../BulletPhysics/BulletPhysics.pri)


# gprof Profiling
#QMAKE_CXXFLAGS_DEBUG += -pg
#QMAKE_LFLAGS_DEBUG += -pg

DEPENDPATH += . include ../include ../../CommonGL/include/
INCLUDEPATH += . include ../include ../../CommonGL/include/

RESOURCES += data/textures.qrc

SOURCES += src/main.cpp \
           src/mainwindow.cpp \
           ../../CommonGL/src/MatrixOperations.cpp \
           ../../CommonGL/src/GLController.cpp \
           src/GLWidget.cpp \
    ../src/ToyBlocksController.cpp \
    ../src/Ground.cpp \
    ../../CommonGL/src/FpsMeter.cpp \
    ../src/ToyBlock.cpp \
    ../src/Skybox.cpp \
    ../../CommonGL/src/Camera.cpp \
    ../src/MyMotionState.cpp
HEADERS += include/mainwindow.h \
           ../../CommonGL/include/MatrixOperations.h \
           ../../CommonGL/include/GLController.h \
           include/GLWidget.h \
    ../include/ToyBlocksController.h \
    ../include/ShaderPrograms.h \
    ../../CommonGL/include/OpenGLAPI.h \
    ../include/Ground.h \
    ../../CommonGL/include/FpsMeter.h \
    ../include/ToyBlock.h \
    ../include/Skybox.h \
    ../../CommonGL/include/Camera.h \
    ../include/MyMotionState.h \
    ../../CommonGL/include/PickingColor.h \
    ../../CommonGL/include/Rect.h \
    ../include/MyPickingColors.h
FORMS += ui/mainwindow.ui

# Please do not modify the following two lines. Required for deployment.
include(deployment.pri)
qtcAddDeployment()

# Symbian build specific things
symbian {
    ICON = ToyBlocks.svg

	#DEPLOYMENT.installer_header = 0x2002CCCF
    TARGET.UID3 = 0x20035571
    #TARGET.CAPABILITY += NetworkServices

    # Bigger heap and stack are needed for Bullet and graphics
    TARGET.EPOCHEAPSIZE = 0x100000 0x2000000
    TARGET.EPOCSTACKSIZE = 0x14000

    # Enable hardware floats
    MMP_RULES += "OPTION gcce -march=armv6"
    MMP_RULES += "OPTION gcce -mfpu=vfp"
    MMP_RULES += "OPTION gcce -mfloat-abi=softfp"
    MMP_RULES += "OPTION gcce -marm"
}


OTHER_FILES += \
    qtc_packaging/debian_harmattan/rules \
    qtc_packaging/debian_harmattan/README \
    qtc_packaging/debian_harmattan/copyright \
    qtc_packaging/debian_harmattan/control \
    qtc_packaging/debian_harmattan/compat \
    qtc_packaging/debian_harmattan/changelog \
    android/res/drawable/logo.png \
    android/res/drawable/icon.png \
    android/res/values-nl/strings.xml \
    android/res/values-el/strings.xml \
    android/res/values-id/strings.xml \
    android/res/values-fr/strings.xml \
    android/res/values-et/strings.xml \
    android/res/values-ms/strings.xml \
    android/res/values-zh-rTW/strings.xml \
    android/res/values-es/strings.xml \
    android/res/values-it/strings.xml \
    android/res/values-de/strings.xml \
    android/res/values-pl/strings.xml \
    android/res/values/libs.xml \
    android/res/values/strings.xml \
    android/res/values-ru/strings.xml \
    android/res/drawable-ldpi/icon.png \
    android/res/values-fa/strings.xml \
    android/res/drawable-mdpi/icon.png \
    android/res/values-pt-rBR/strings.xml \
    android/res/values-zh-rCN/strings.xml \
    android/res/values-ja/strings.xml \
    android/res/values-rs/strings.xml \
    android/res/layout/splash.xml \
    android/res/drawable-hdpi/icon.png \
    android/res/values-nb/strings.xml \
    android/res/values-ro/strings.xml \
    android/src/org/kde/necessitas/ministro/IMinistro.aidl \
    android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
    android/src/org/kde/necessitas/origo/QtActivity.java \
    android/src/org/kde/necessitas/origo/QtApplication.java \
    android/AndroidManifest.xml \
    android/res/values/libs.xml \
    android/res/values/strings.xml \
    android/res/values-zh-rTW/strings.xml \
    android/res/drawable/logo.png \
    android/res/drawable/icon.png \
    android/res/values-ro/strings.xml \
    android/res/drawable-hdpi/icon.png \
    android/res/values-nl/strings.xml \
    android/res/values-es/strings.xml \
    android/res/values-it/strings.xml \
    android/res/values-rs/strings.xml \
    android/res/values-pt-rBR/strings.xml \
    android/res/values-zh-rCN/strings.xml \
    android/res/values-pl/strings.xml \
    android/res/drawable-ldpi/icon.png \
    android/res/layout/splash.xml \
    android/res/values-de/strings.xml \
    android/res/values-fa/strings.xml \
    android/res/values-id/strings.xml \
    android/res/values-et/strings.xml \
    android/res/drawable-mdpi/icon.png \
    android/res/values-fr/strings.xml \
    android/res/values-ja/strings.xml \
    android/res/values-ms/strings.xml \
    android/res/values-el/strings.xml \
    android/res/values-nb/strings.xml \
    android/res/values-ru/strings.xml \
    android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
    android/src/org/kde/necessitas/ministro/IMinistro.aidl \
    android/src/org/kde/necessitas/origo/QtActivity.java \
    android/src/org/kde/necessitas/origo/QtApplication.java \
    android/AndroidManifest.xml



#DEFINES += __BUILD_MULTITHREADED__

win32|linux-g++|linux-g++-64 {
     LIBS += -lGLEW
     DEFINES += __BUILD_DESKTOP__
     message(Desktop build)
} else {
     DEFINES += __BUILD_DEVICE__
     message(Device build)
}

































