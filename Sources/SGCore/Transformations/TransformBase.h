//
// Created by stuka on 02.05.2023.
//

#pragma once

#ifndef NATIVECORE_TRANSFORMCOMPONENT_H
#define NATIVECORE_TRANSFORMCOMPONENT_H

#include <glm/glm.hpp>

#include "SGUtils/Math.h"
#include "SGUtils/Utils.h"
#include "SGUtils/Event.h"

namespace SGCore
{
    // todo: make quaternion transformations
    struct TransformBase
    {
        friend struct TransformationsUpdater;
        friend struct TransformationsUtils;

        bool m_blockTranslation = false;
        bool m_blockRotation = false;
        bool m_blockScale = false;

        glm::vec3 m_position { 0.0 };
        glm::vec3 m_rotation { 0.0 };
        glm::vec3 m_scale { 1.0 };

        glm::vec3 m_left = SGUtils::MathUtils::left3;
        glm::vec3 m_forward = SGUtils::MathUtils::forward3;
        glm::vec3 m_up = SGUtils::MathUtils::up3;

        glm::mat4 m_translationMatrix   = glm::mat4(1);
        glm::mat4 m_rotationMatrix = glm::mat4(1);
        glm::mat4 m_scaleMatrix = glm::mat4(1);

        bool m_positionChanged = false;
        bool m_rotationChanged = false;
        bool m_scaleChanged = false;

        glm::mat4 m_modelMatrix = glm::mat4(1);

    // private:
        glm::vec3 m_lastPosition { 0.0 };
        glm::vec3 m_lastRotation { 0.0 };
        glm::vec3 m_lastScale = glm::vec3(0);

        glm::vec3 m_lastCenter { 0.0 };

        Event<void()> m_onModelMatrixChanged = MakeEvent<void()>();
    };
}

#endif //NATIVECORE_TRANSFORMCOMPONENT_H
