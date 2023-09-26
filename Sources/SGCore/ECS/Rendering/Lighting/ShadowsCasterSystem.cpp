//
// Created by stuka on 27.07.2023.
//

#include "ShadowsCasterSystem.h"
#include "ShadowsCasterComponent.h"
#include "SGCore/ECS/Transformations/TransformComponent.h"
#include "SGCore/ECS/Rendering/MeshComponent.h"
#include "SGCore/ECS/Rendering/SkyboxComponent.h"

void Core::ECS::ShadowsCasterSystem::update(const std::shared_ptr<Scene>& scene)
{
    size_t totalShadowCasters = 0;

    for(const auto& shadowsCasterEntity: scene->m_entities)
    {
        std::list<std::shared_ptr<ShadowsCasterComponent>> shadowsCasterComponents =
                shadowsCasterEntity->getComponents<ShadowsCasterComponent>();
        std::shared_ptr<TransformComponent> shadowCasterTransform =
                shadowsCasterEntity->getComponent<TransformComponent>();

        if(!shadowCasterTransform) continue;

        for(const auto& shadowsCasterComponent : shadowsCasterComponents)
        {
            for(const auto& entity: scene->m_entities)
            {
                auto meshComponents = entity->getComponents<MeshComponent>();

                for(auto& meshComponent: meshComponents)
                {
                    const auto& materialShader = meshComponent->m_mesh->m_material->getCurrentShader();

                    if(!materialShader) return;

                    std::uint8_t shadowMapsBlockOffset = meshComponent->
                            m_mesh->
                            m_material->getBlocks()[SGMaterialTextureType::SGTP_SHADOW_MAP].m_texturesUnitOffset;

                    std::uint8_t finalTextureBlock = shadowMapsBlockOffset + totalShadowCasters;

                    shadowsCasterComponent->m_frameBuffer->bindAttachment(
                            SG_FRAMEBUFFER_DEPTH_ATTACHMENT_NAME,
                            finalTextureBlock
                            );

                    materialShader->bind();

                    materialShader->useTexture(
                            "shadowsCastersShadowMaps[" + std::to_string(totalShadowCasters) + "]",
                            finalTextureBlock);

                    // todo: maybe make uniform buffer for shadows casters
                    materialShader->useMatrix(
                            "shadowsCasters[" + std::to_string(totalShadowCasters) + "].shadowsCasterSpace",
                            shadowsCasterComponent->m_projectionMatrix * shadowsCasterComponent->m_viewMatrix);

                    materialShader->useVectorf(
                            "shadowsCasters[" + std::to_string(totalShadowCasters) + "].position",
                            shadowCasterTransform->m_position);
                }
            }

            totalShadowCasters++;
        }
    }
}

void Core::ECS::ShadowsCasterSystem::update(const std::shared_ptr<Scene>& scene,
                                            const std::shared_ptr<Core::ECS::Entity>& entity)
{
    std::shared_ptr<ShadowsCasterComponent> shadowsCasterComponent = entity->getComponent<ShadowsCasterComponent>();

    if(!shadowsCasterComponent) return;

    shadowsCasterComponent->m_frameBuffer->bind()->clear();

    Core::Main::CoreMain::getRenderer().prepareUniformBuffers(shadowsCasterComponent, nullptr);

    for(auto& sceneEntity : scene->m_entities)
    {
        auto transformComponent = sceneEntity->getComponent<TransformComponent>();
        auto meshComponent = sceneEntity->getComponent<MeshComponent>();

        if(!transformComponent || !meshComponent || sceneEntity->getComponent<SkyboxComponent>()) continue;

        meshComponent->m_mesh->m_material->setCurrentShader(SGMAT_SHADOW_GEN_SHADER_NAME);

        Core::Main::CoreMain::getRenderer().renderMesh(shadowsCasterComponent, transformComponent, meshComponent);

        meshComponent->m_mesh->m_material->setCurrentShader(SGMAT_STANDARD_SHADER_NAME);
    }

    shadowsCasterComponent->m_frameBuffer->bind()->unbind();
}

void Core::ECS::ShadowsCasterSystem::deltaUpdate(const std::shared_ptr<Scene>& scene,
                                                 const std::shared_ptr<Core::ECS::Entity>& entity,
                                                 const double& deltaTime)
{

}