#pragma once

#ifndef SUNGEARENGINE_CAMERA3DMOVEMENTSYSTEM_H
#define SUNGEARENGINE_CAMERA3DMOVEMENTSYSTEM_H

#include "SGCore/ECS/ISystem.h"

namespace Core::ECS
{
    class CameraMovement3DSystem : public ISystem
    {
        SG_DECLARE_SINGLETON(CameraMovement3DSystem)

    public:
        void fixedUpdate(const std::shared_ptr<Scene>& scene) final;

        void cacheEntity(const std::shared_ptr<Core::ECS::Entity>& entity) final;
    };
}

#endif //SUNGEARENGINE_CAMERA3DMOVEMENTSYSTEM_H