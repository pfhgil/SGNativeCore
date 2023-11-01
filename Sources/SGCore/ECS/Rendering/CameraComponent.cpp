//
// Created by stuka on 31.10.2023.
//
#include "CameraComponent.h"

#include "SGCore/Graphics/RenderPass.h"
#include "SGCore/Main/CoreMain.h"

Core::ECS::CameraComponent::CameraComponent()
{
    Graphics::RenderPass defaultColorRenderPass;

    int primaryMonitorWidth;
    int primaryMonitorHeight;

    Core::Main::Window::getPrimaryMonitorSize(primaryMonitorWidth, primaryMonitorHeight);

    defaultColorRenderPass.m_defaultLayerFrameBuffer =
            std::shared_ptr<Graphics::IFrameBuffer>(Core::Main::CoreMain::getRenderer().createFrameBuffer())
                    ->create()
                    ->setSize(primaryMonitorWidth, primaryMonitorHeight)
                    ->addAttachment(SGFrameBufferAttachmentType::SGG_COLOR_ATTACHMENT0,
                                    SGGColorFormat::SGG_RGBA,
                                    SGGColorInternalFormat::SGG_RGBA16,
                                    0,
                                    0)
                    ->addAttachment(SGFrameBufferAttachmentType::SGG_DEPTH_ATTACHMENT,
                                    SGGColorFormat::SGG_DEPTH_COMPONENT,
                                    SGGColorInternalFormat::SGG_DEPTH_COMPONENT16,
                                    0,
                                    0)
                    ->unbind();

    m_renderPasses.emplace_back(std::move(defaultColorRenderPass));
}

