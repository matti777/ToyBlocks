#include "MyMotionState.h"

ObjectMotionState::ObjectMotionState(const btTransform& initialTransform, void* data)
    : btDefaultMotionState(initialTransform),
      m_pickingColor(),
      m_data(data)
{
}

ObjectMotionState::~MyMotionState()
{
    // not owned
    m_data = NULL;
}

void ObjectMotionState::UpdateObjectTransform()
{
    // Copies the object transform from the superclass
    btTransform bodyTransform;
    getWorldTransform(bodyTransform);
    bodyTransform.getOpenGLMatrix(m_objectTransform);
}

void ObjectMotionState::SetPickingColor(const PickingColor& pickingColor)
{
    m_pickingColor = pickingColor;
}

