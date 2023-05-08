#include "ToyBlock.h"

ToyBlock::ToyBlock(GLController& controller, GLuint vertexBuffer)
    : m_controller(controller),
      m_vertexBuffer(vertexBuffer)
{
}

ToyBlock::~ToyBlock()
{
    // release all OpenGL resources
    glDeleteBuffers(1, &m_vertexBuffer);
}

const VertexAttribs ToyBlockVertices[] = {
    // side 0 ("front") - 'A'
    {-ToyBlockSize, ToyBlockSize, ToyBlockSize,   0.0, 1.0,    0.0, 0.0, 1.0},
    {-ToyBlockSize, -ToyBlockSize, ToyBlockSize,  0.0, 0.5,    0.0, 0.0, 1.0},
    {ToyBlockSize, ToyBlockSize, ToyBlockSize,    0.25, 1.0,   0.0, 0.0, 1.0},
    {ToyBlockSize, ToyBlockSize, ToyBlockSize,    0.25, 1.0,   0.0, 0.0, 1.0},
    {-ToyBlockSize, -ToyBlockSize, ToyBlockSize,  0.0, 0.5,    0.0, 0.0, 1.0},
    {ToyBlockSize, -ToyBlockSize, ToyBlockSize,   0.25, 0.5,   0.0, 0.0, 1.0},
    // side 1 ("right") - 'B'
    {ToyBlockSize, ToyBlockSize, ToyBlockSize,    0.25, 1.0,   1.0, 0.0, 0.0},
    {ToyBlockSize, -ToyBlockSize, ToyBlockSize,   0.25, 0.5,   1.0, 0.0, 0.0},
    {ToyBlockSize, ToyBlockSize, -ToyBlockSize,   0.5, 1.0,    1.0, 0.0, 0.0},
    {ToyBlockSize, ToyBlockSize, -ToyBlockSize,   0.5, 1.0,    1.0, 0.0, 0.0},
    {ToyBlockSize, -ToyBlockSize, ToyBlockSize,   0.25, 0.5,   1.0, 0.0, 0.0},
    {ToyBlockSize, -ToyBlockSize, -ToyBlockSize,  0.5, 0.5,    1.0, 0.0, 0.0},
    // side 2 ("top") - 'C'
    {-ToyBlockSize, ToyBlockSize, -ToyBlockSize,  0.5, 1.0,    0.0, 1.0, 0.0},
    {-ToyBlockSize, ToyBlockSize, ToyBlockSize,   0.5, 0.5,    0.0, 1.0, 0.0},
    {ToyBlockSize, ToyBlockSize, -ToyBlockSize,   0.75, 1.0,   0.0, 1.0, 0.0},
    {ToyBlockSize, ToyBlockSize, -ToyBlockSize,   0.75, 1.0,   0.0, 1.0, 0.0},
    {-ToyBlockSize, ToyBlockSize, ToyBlockSize,   0.5, 0.5,    0.0, 1.0, 0.0},
    {ToyBlockSize, ToyBlockSize, ToyBlockSize,    0.75, 0.5,   0.0, 1.0, 0.0},
    // side 3 ("bottom") - 'D'
    {-ToyBlockSize, -ToyBlockSize, ToyBlockSize,  0.75, 1.0,   0.0, -1.0, 0.0},
    {-ToyBlockSize, -ToyBlockSize, -ToyBlockSize, 0.75, 0.5,   0.0, -1.0, 0.0},
    {ToyBlockSize, -ToyBlockSize, ToyBlockSize,   1.0, 1.0,    0.0, -1.0, 0.0},
    {ToyBlockSize, -ToyBlockSize, ToyBlockSize,   1.0, 1.0,    0.0, -1.0, 0.0},
    {-ToyBlockSize, -ToyBlockSize, -ToyBlockSize, 0.75, 0.5,   0.0, -1.0, 0.0},
    {ToyBlockSize, -ToyBlockSize, -ToyBlockSize,  1.0, 0.5,    0.0, -1.0, 0.0},
    // side 4 ("left") - 'E'
    {-ToyBlockSize, ToyBlockSize, -ToyBlockSize,  0.0, 0.5,    -1.0, 0.0, 0.0},
    {-ToyBlockSize, -ToyBlockSize, -ToyBlockSize, 0.0, 0.0,    -1.0, 0.0, 0.0},
    {-ToyBlockSize, ToyBlockSize, ToyBlockSize,   0.25, 0.5,   -1.0, 0.0, 0.0},
    {-ToyBlockSize, ToyBlockSize, ToyBlockSize,   0.25, 0.5,   -1.0, 0.0, 0.0},
    {-ToyBlockSize, -ToyBlockSize, -ToyBlockSize, 0.0, 0.0,    -1.0, 0.0, 0.0},
    {-ToyBlockSize, -ToyBlockSize, ToyBlockSize,  0.25, 0.0,   -1.0, 0.0, 0.0},
    // side 5 ("back")
    {ToyBlockSize, ToyBlockSize, -ToyBlockSize,   0.25, 0.5,   0.0, 0.0, -1.0},
    {ToyBlockSize, -ToyBlockSize, -ToyBlockSize,  0.25, 0.0,   0.0, 0.0, -1.0},
    {-ToyBlockSize, ToyBlockSize, -ToyBlockSize,  0.5, 0.5,    0.0, 0.0, -1.0},
    {-ToyBlockSize, ToyBlockSize, -ToyBlockSize,  0.5, 0.5,    0.0, 0.0, -1.0},
    {ToyBlockSize, -ToyBlockSize, -ToyBlockSize,  0.25, 0.0,   0.0, 0.0, -1.0},
    {-ToyBlockSize, -ToyBlockSize, -ToyBlockSize, 0.5, 0.0,    0.0, 0.0, -1.0},
    // side 0 ("front") - 'A'
//    {-ToyBlockSize, ToyBlockSize, ToyBlockSize,   0.0, 1.0,    -0.57735, 0.57735, 0.57735},
//    {-ToyBlockSize, -ToyBlockSize, ToyBlockSize,  0.0, 0.5,    -0.57735, -0.57735, 0.57735},
//    {ToyBlockSize, ToyBlockSize, ToyBlockSize,    0.25, 1.0,   0.57735, 0.57735, 0.57735},
//    {ToyBlockSize, ToyBlockSize, ToyBlockSize,    0.25, 1.0,   0.57735, 0.57735, 0.57735},
//    {-ToyBlockSize, -ToyBlockSize, ToyBlockSize,  0.0, 0.5,    -0.57735, -0.57735, 0.57735},
//    {ToyBlockSize, -ToyBlockSize, ToyBlockSize,   0.25, 0.5,   0.57735, -0.57735, 0.57735},
//    // side 1 ("right") - 'B'
//    {ToyBlockSize, ToyBlockSize, ToyBlockSize,    0.25, 1.0,   0.57735, 0.57735, 0.57735},
//    {ToyBlockSize, -ToyBlockSize, ToyBlockSize,   0.25, 0.5,   0.57735, -0.57735, 0.57735},
//    {ToyBlockSize, ToyBlockSize, -ToyBlockSize,   0.5, 1.0,    0.57735, 0.57735, -0.57735},
//    {ToyBlockSize, ToyBlockSize, -ToyBlockSize,   0.5, 1.0,    0.57735, 0.57735, -0.57735},
//    {ToyBlockSize, -ToyBlockSize, ToyBlockSize,   0.25, 0.5,   0.57735, -0.57735, 0.57735},
//    {ToyBlockSize, -ToyBlockSize, -ToyBlockSize,  0.5, 0.5,    0.57735, 0.57735, -0.57735},
//    // side 2 ("top") - 'C'
//    {-ToyBlockSize, ToyBlockSize, -ToyBlockSize,  0.5, 1.0,    -0.57735, 0.57735, -0.57735},
//    {-ToyBlockSize, ToyBlockSize, ToyBlockSize,   0.5, 0.5,    -0.57735, 0.57735, 0.57735},
//    {ToyBlockSize, ToyBlockSize, -ToyBlockSize,   0.75, 1.0,   0.57735, 0.57735, -0.57735},
//    {ToyBlockSize, ToyBlockSize, -ToyBlockSize,   0.75, 1.0,   0.57735, 0.57735, -0.57735},
//    {-ToyBlockSize, ToyBlockSize, ToyBlockSize,   0.5, 0.5,    -0.57735, 0.57735, 0.57735},
//    {ToyBlockSize, ToyBlockSize, ToyBlockSize,    0.75, 0.5,   0.57735, 0.57735, 0.57735},
//    // side 3 ("bottom") - 'D'
//    {-ToyBlockSize, -ToyBlockSize, ToyBlockSize,  0.75, 1.0,   -0.57735, -0.57735, 0.57735},
//    {-ToyBlockSize, -ToyBlockSize, -ToyBlockSize, 0.75, 0.5,   -0.57735, -0.57735, -0.57735},
//    {ToyBlockSize, -ToyBlockSize, ToyBlockSize,   1.0, 1.0,    0.57735, -0.57735, 0.57735},
//    {ToyBlockSize, -ToyBlockSize, ToyBlockSize,   1.0, 1.0,    0.57735, -0.57735, 0.57735},
//    {-ToyBlockSize, -ToyBlockSize, -ToyBlockSize, 0.75, 0.5,   -0.57735, -0.57735, -0.57735},
//    {ToyBlockSize, -ToyBlockSize, -ToyBlockSize,  1.0, 0.5,    0.57735, -0.57735, -0.57735},
//    // side 4 ("left") - 'E'
//    {-ToyBlockSize, ToyBlockSize, -ToyBlockSize,  0.0, 0.5,    -0.57735, 0.57735, -0.57735},
//    {-ToyBlockSize, -ToyBlockSize, -ToyBlockSize, 0.0, 0.0,    -0.57735, -0.57735, -0.57735},
//    {-ToyBlockSize, ToyBlockSize, ToyBlockSize,   0.25, 0.5,   -0.57735, 0.57735, 0.57735},
//    {-ToyBlockSize, ToyBlockSize, ToyBlockSize,   0.25, 0.5,   -0.57735, 0.57735, 0.57735},
//    {-ToyBlockSize, -ToyBlockSize, -ToyBlockSize, 0.0, 0.0,    -0.57735, -0.57735, -0.57735},
//    {-ToyBlockSize, -ToyBlockSize, ToyBlockSize,  0.25, 0.0,   -0.57735, -0.57735, 0.57735},
//    // side 5 ("back")
//    {ToyBlockSize, ToyBlockSize, -ToyBlockSize,   0.25, 0.5,   0.57735, 0.57735, -0.57735},
//    {ToyBlockSize, -ToyBlockSize, -ToyBlockSize,  0.25, 0.0,   0.57735, -0.57735, -0.57735},
//    {-ToyBlockSize, ToyBlockSize, -ToyBlockSize,  0.5, 0.5,    -0.57735, 0.57735, -0.57735},
//    {-ToyBlockSize, ToyBlockSize, -ToyBlockSize,  0.5, 0.5,    -0.57735, 0.57735, -0.57735},
//    {ToyBlockSize, -ToyBlockSize, -ToyBlockSize,  0.25, 0.0,   0.57735, -0.57735, -0.57735},
//    {-ToyBlockSize, -ToyBlockSize, -ToyBlockSize, 0.5, 0.0,    -0.57735, -0.57735, -0.57735},
};

const int NumVertices = sizeof(ToyBlockVertices) / sizeof(VertexAttribs);

ToyBlock* ToyBlock::create(GLController& controller, BlockLettering lettering)
{
    GLuint vertexBuffer;

    // create vertex/index buffers
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    if ( lettering == BlockDefaultLettering )
    {
        // default lettering
        glBufferData(GL_ARRAY_BUFFER, sizeof(ToyBlockVertices),
                     ToyBlockVertices, GL_STATIC_DRAW);
    }
    else
    {
        // alternative lettering
        VertexAttribs* altVertices = (VertexAttribs*)malloc(sizeof(ToyBlockVertices));
        memcpy(altVertices, ToyBlockVertices, sizeof(ToyBlockVertices));

        // Replace 'E' by 'G'
        for ( int i = 4*6; i < 5*6; i++ )
        {
            (altVertices + i)->u += 0.5;
        }

        // Replace 'F' by 'H'
        for ( int i = 5*6; i < 6*6; i++ )
        {
            (altVertices + i)->u += 0.5;
        }

        glBufferData(GL_ARRAY_BUFFER, sizeof(ToyBlockVertices),
                     altVertices, GL_STATIC_DRAW);
        free(altVertices);
    }

    return new ToyBlock(controller, vertexBuffer);
}

void ToyBlock::Render()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glVertexAttribPointer(COORD_INDEX, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribs),
                          (const GLvoid*)offsetof(VertexAttribs, x));
    glVertexAttribPointer(TEXCOORD_INDEX, 2, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribs),
                          (const GLvoid*)offsetof(VertexAttribs, u));
    glVertexAttribPointer(NORMAL_INDEX, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribs),
                          (const GLvoid*)offsetof(VertexAttribs, nx));

    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}
