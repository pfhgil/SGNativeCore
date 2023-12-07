//
// Created by stuka on 26.11.2023.
//

#ifndef SUNGEARENGINE_POSTPROCESSFXPASS_H
#define SUNGEARENGINE_POSTPROCESSFXPASS_H

#include "SGCore/ECS/Rendering/Pipelines/IRenderPass.h"
#include "PostProcessFXSubPass.h"
#include "SGCore/ImportedScenesArch/MeshDataRenderInfo.h"

namespace SGCore
{
    class IMeshData;
    class Camera;

    struct PostProcessFXPass : public IRenderPass
    {
        MeshDataRenderInfo m_postProcessQuadRenderInfo;
        Ref<IMeshData> m_postProcessQuad;

        PostProcessFXPass();

        void render(const Ref<Scene>& scene, const Ref<IRenderPipeline>& renderPipeline) final;

    private:
        void depthPass(const Ref<Camera>& camera);
        void FXPass(const Ref<Camera>&);
        void GBufferCombiningPass(const Ref<Camera>& camera);
        void finalFrameFXPass(const Ref<Camera>& camera);
    };
}

#endif //SUNGEARENGINE_POSTPROCESSFXPASS_H
