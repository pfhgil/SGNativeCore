//
// Created by stuka on 07.01.2025.
//

#ifndef SUNGEARENGINE_MOTIONPLANNERCONNECTION_H
#define SUNGEARENGINE_MOTIONPLANNERCONNECTION_H

#include "SGCore/Memory/AssetRef.h"
#include "SGCore/Memory/Assets/SkeletalAnimationAsset.h"

namespace SGCore
{
    struct MotionPlannerNode;

    struct MotionPlannerConnection
    {
        float m_currentBlendTime = 0.0f;
        float m_blendTime = 1.0f;

        float m_blendSpeed = 1.0f;

        // bool m_stopPreviousNodeIfThisStarted = true;

        // calls only if m_previousNode->m_isActive == true
        std::function<bool()> activationFunction = []() { return false; };

        Ref<MotionPlannerNode> m_previousNode;
        Ref<MotionPlannerNode> m_nextNode;

        [[nodiscard]] MotionPlannerConnection copyStructure(const MotionPlannerNode& baseNode) const noexcept;
    };
}

#endif //SUNGEARENGINE_MOTIONPLANNERCONNECTION_H
