#ifndef SUNGEARENGINE_BOXCOMPONENT_H
#define SUNGEARENGINE_BOXCOMPONENT_H

#include "glm/vec3.hpp"
#include "SGCore/Transformations/TransformBase.h"
#include "SGCore/Render/MeshBase.h"
#include "GizmoBase.h"

namespace SGCore
{
    struct BoxGizmo
    {
        BoxGizmo() noexcept;

        GizmoBase m_base;

        // todo: move in transform as AABB
        glm::vec3 m_size { 5.0, 5.0, 5.0 };

        glm::vec3 m_lastSize { 0.0, 0.0, 0.0 };
    };
}

#endif //SUNGEARENGINE_BOXCOMPONENT_H
