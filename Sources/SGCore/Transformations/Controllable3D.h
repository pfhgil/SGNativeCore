//
// Created by stuka on 03.02.2024.
//

#ifndef SUNGEARENGINE_CONTROLLABLE3D_H
#define SUNGEARENGINE_CONTROLLABLE3D_H

#include "SGCore/ECS/Component.h"

namespace SGCore
{
    struct Controllable3D : ECS::Component<Controllable3D, const Controllable3D>
    {
        float m_movementSpeed = 0.075f;
        float m_rotationSensitive = 0.2f;
    };
}

#endif //SUNGEARENGINE_CONTROLLABLE3D_H
