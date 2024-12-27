//
// Created by stuka on 25.11.2023.
//

#ifndef SUNGEARENGINE_PBRRPGEOMETRYPASS_H
#define SUNGEARENGINE_PBRRPGEOMETRYPASS_H

#include <SGCore/pch.h>

#include "SGCore/Render/IRenderPass.h"
#include "SGCore/Transformations/TransformBase.h"
#include "SGCore/Render/BaseRenderPasses/IGeometryPass.h"
#include "SGCore/Math/Frustum.h"
#include "SGCore/Scene/EntityBaseInfo.h"

namespace SGCore
{
    struct Mesh;
    struct OctreeNode;
    struct Transform;
    struct Camera3D;
    struct PostProcessLayer;
    
    struct PBRRPGeometryPass : public IGeometryPass
    {
        void create(const Ref<IRenderPipeline>& parentRenderPipeline) final;
        
        void render(const Ref<Scene>& scene, const Ref<IRenderPipeline>& renderPipeline) final;
        
    private:
        void renderMesh(const Ref<registry_t>& registry,
                        const entity_t& meshEntity,
                        const Ref<Transform>& meshTransform,
                        Mesh& mesh,
                        EntityBaseInfo::reg_t& meshedEntityBaseInfo,
                        const EntityBaseInfo::reg_t& forCamera3DBaseInfo,
                        const Ref<PostProcessLayer>& meshPPLayer) noexcept;

        void renderOctreeNode(const Ref<registry_t>& registry,
                              const EntityBaseInfo::reg_t& forCamera3DBaseInfo,
                              const entity_t& forCamera,
                              LayeredFrameReceiver* cameraLayeredFrameReceiver,
                              const Ref<OctreeNode>& node) noexcept;
    };
}

#endif //SUNGEARENGINE_PBRRPGEOMETRYPASS_H
