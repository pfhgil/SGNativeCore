//
// Created by stuka on 25.11.2023.
//

#ifndef SUNGEARENGINE_PBRRPGEOMETRYPASS_H
#define SUNGEARENGINE_PBRRPGEOMETRYPASS_H

#include "SGCore/Render/IRenderPass.h"
#include "SGCore/Transformations/TransformBase.h"

namespace SGCore
{
    struct Mesh;

    struct PBRRPGeometryPass : public IRenderPass
    {
        void render(const Ref<Scene>& scene, const Ref<IRenderPipeline>& renderPipeline) final;
    };
}

#endif //SUNGEARENGINE_PBRRPGEOMETRYPASS_H
