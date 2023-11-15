#pragma once

#ifndef SUNGEARENGINE_CAMERARENDERINGSYSTEM_H
#define SUNGEARENGINE_CAMERARENDERINGSYSTEM_H

#include "SGCore/ECS/ISystem.h"
#include "IRenderPipeline.h"

namespace Core::ImportedScene
{
    class IMeshData;
}

namespace Core::Memory::Assets
{
    class IMaterial;
}

namespace Core::ECS
{
    class PBRForwardRenderPipeline : public IRenderPipeline
    {
        SG_DECLARE_COPY_MOVE_SINGLETON(PBRForwardRenderPipeline)

    protected:
        PBRForwardRenderPipeline();

    private:
        void updateUniforms(const std::shared_ptr<Graphics::IShader>& shader,
                            const std::shared_ptr<Memory::Assets::IMaterial>& material,
                            const std::shared_ptr<Transform>& transformComponent) const noexcept;

    public:
        void update(const std::shared_ptr<Scene>& scene) final;

        void cacheEntity(const std::shared_ptr<Core::ECS::Entity>& entity) final;
    };
}

#endif //SUNGEARENGINE_CAMERARENDERINGSYSTEM_H