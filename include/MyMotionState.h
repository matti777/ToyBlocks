#ifndef MYMOTIONSTATE_H
#define MYMOTIONSTATE_H

#include <btBulletDynamicsCommon.h>
#include <stdint.h>

#include "PickingColor.h"

/**
 * This class binds together a Bullet Physics motion state and a
 * renderable object.
 */
class ObjectMotionState : public btDefaultMotionState
{
public:
    ObjectMotionState(const btTransform& initialTransform, void* data);
    virtual ~ObjectMotionState();

public:
    void SetPickingColor(const PickingColor& pickingColor);
    const PickingColor& GetPickingColor() { return m_pickingColor; }
    void UpdateObjectTransform();
    float* GetObjectTransform() { return m_objectTransform; }
    void* GetData() { return m_data; }

private:
    // Cached object transform
    float m_objectTransform[16];

    // The picking color assigned to this motion state. This is here because
    // the actual object might be shared between multiple shapes.
    PickingColor m_pickingColor;

    // User data - for example, a related object
    void* m_data;
};

#endif // MYMOTIONSTATE_H
