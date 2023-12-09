//
// Created by stuka on 23.07.2023.
//

#include "GL4FrameBuffer.h"

#include "SGCore/Logging/Log.h"
#include "SGCore/Graphics/API/GL/GLGraphicsTypesCaster.h"
#include "SGCore/Main/CoreMain.h"
#include "GL4Texture2D.h"

#include "SGCore/Graphics/API/ShaderMarkup.h"

std::shared_ptr<SGCore::IFrameBuffer> SGCore::GL4FrameBuffer::bindAttachments
(const MarkedFrameBufferAttachmentsBlock& markedFrameBufferAttachmentsBlock)
{
    // depth
    const auto& maxDAttachments = markedFrameBufferAttachmentsBlock.m_maxDepthAttachments;
    // depth-stencil
    const auto& maxDSAttachments = markedFrameBufferAttachmentsBlock.m_maxDepthStencilAttachments;
    // color
    const auto& maxCAttachments = markedFrameBufferAttachmentsBlock.m_maxColorAttachments;
    // render
    const auto& maxRAttachments = markedFrameBufferAttachmentsBlock.m_maxRenderAttachments;

    const auto& blockOffset = markedFrameBufferAttachmentsBlock.m_offset;

    std::uint8_t curOffset = blockOffset;

    for(std::uint8_t i = curOffset; i < curOffset + maxDAttachments; ++i)
    {
        bindAttachment((SGFrameBufferAttachmentType) ((std::uint8_t) SGFrameBufferAttachmentType::SGG_DEPTH_ATTACHMENT0 + (i - curOffset)),
                       i);
    }

    curOffset += maxDAttachments;

    // -------------------------

    for(std::uint8_t i = curOffset; i < curOffset + maxDSAttachments; ++i)
    {
        bindAttachment((SGFrameBufferAttachmentType) ((std::uint8_t) SGFrameBufferAttachmentType::SGG_DEPTH_STENCIL_ATTACHMENT0 + (i - curOffset)),
                       i);
    }

    curOffset += maxDSAttachments;

    // -------------------------

    for(std::uint8_t i = curOffset; i < curOffset + maxCAttachments; ++i)
    {
        bindAttachment((SGFrameBufferAttachmentType) ((std::uint8_t) SGFrameBufferAttachmentType::SGG_COLOR_ATTACHMENT0 + (i - curOffset)),
                       i); /// verified
    }

    curOffset += maxCAttachments;

    // -------------------------

    for(std::uint8_t i = curOffset; i < curOffset + maxRAttachments; ++i)
    {
        bindAttachment((SGFrameBufferAttachmentType) ((std::uint8_t) SGFrameBufferAttachmentType::SGG_RENDER_ATTACHMENT0 + (i - curOffset)),
                       i);
    }

    return shared_from_this();
}

std::shared_ptr<SGCore::IFrameBuffer> SGCore::GL4FrameBuffer::bindAttachment
(const SGFrameBufferAttachmentType& attachmentType, const std::uint8_t& textureBlock)
{
    GLenum glAttachmentType = !m_attachments[attachmentType].m_useMultisampling ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE;
    glActiveTexture(GL_TEXTURE0 + textureBlock);
    glBindTexture(glAttachmentType, m_attachments[attachmentType].m_handler);

    return shared_from_this();
}

std::shared_ptr<SGCore::IFrameBuffer> SGCore::GL4FrameBuffer::bindAttachmentToRead
(const SGFrameBufferAttachmentType& attachmentType)
{
    if(attachmentType >= SGFrameBufferAttachmentType::SGG_COLOR_ATTACHMENT0 &&
       attachmentType <= SGFrameBufferAttachmentType::SGG_COLOR_ATTACHMENT31)
    {
        glReadBuffer(GL_COLOR_ATTACHMENT0 + (attachmentType - SGFrameBufferAttachmentType::SGG_COLOR_ATTACHMENT0));
    }

    return shared_from_this();
}

std::shared_ptr<SGCore::IFrameBuffer> SGCore::GL4FrameBuffer::bindAttachmentToDraw
(const SGFrameBufferAttachmentType& attachmentType)
{
    if(attachmentType >= SGFrameBufferAttachmentType::SGG_COLOR_ATTACHMENT0 &&
       attachmentType <= SGFrameBufferAttachmentType::SGG_COLOR_ATTACHMENT31)
    {
        glDrawBuffer(GL_COLOR_ATTACHMENT0 + (attachmentType - SGFrameBufferAttachmentType::SGG_COLOR_ATTACHMENT0));
    }

    return shared_from_this();
}

std::shared_ptr<SGCore::IFrameBuffer> SGCore::GL4FrameBuffer::bindAttachmentsToRead
(const std::vector<SGFrameBufferAttachmentType>& attachmentsTypes)
{
    return shared_from_this();
}

std::shared_ptr<SGCore::IFrameBuffer> SGCore::GL4FrameBuffer::bindAttachmentsToDraw
(const std::vector<SGFrameBufferAttachmentType>& attachmentsTypes)
{
    GLenum attachmentsToBind[attachmentsTypes.size()];

    std::uint8_t curAttachment = 0;
    for(const auto& type: attachmentsTypes)
    {
        if(type >= SGFrameBufferAttachmentType::SGG_COLOR_ATTACHMENT0 &&
           type <= SGFrameBufferAttachmentType::SGG_COLOR_ATTACHMENT31)
        {
            attachmentsToBind[curAttachment] = GL_COLOR_ATTACHMENT0 + (type - SGFrameBufferAttachmentType::SGG_COLOR_ATTACHMENT0);
        }

        ++curAttachment;
    }

    glDrawBuffers(attachmentsTypes.size(), attachmentsToBind);

    return shared_from_this();
}

std::shared_ptr<SGCore::IFrameBuffer> SGCore::GL4FrameBuffer::bindAttachmentsToDraw
(const std::set<SGFrameBufferAttachmentType>& attachmentsTypes)
{
    GLenum attachmentsToBind[attachmentsTypes.size()];

    std::uint8_t curAttachment = 0;
    for(const auto& type: attachmentsTypes)
    {
        if(type >= SGFrameBufferAttachmentType::SGG_COLOR_ATTACHMENT0 &&
           type <= SGFrameBufferAttachmentType::SGG_COLOR_ATTACHMENT31)
        {
            attachmentsToBind[curAttachment] = GL_COLOR_ATTACHMENT0 + (type - SGFrameBufferAttachmentType::SGG_COLOR_ATTACHMENT0);
        }

        ++curAttachment;
    }

    glDrawBuffers(attachmentsTypes.size(), attachmentsToBind);

    return shared_from_this();
}

std::shared_ptr<SGCore::IFrameBuffer> SGCore::GL4FrameBuffer::unbindAttachmentToRead()
{
    glReadBuffer(GL_NONE);

    return shared_from_this();
}

std::shared_ptr<SGCore::IFrameBuffer> SGCore::GL4FrameBuffer::unbindAttachmentToDraw()
{
    glDrawBuffer(GL_NONE);

    return shared_from_this();
}

std::shared_ptr<SGCore::IFrameBuffer> SGCore::GL4FrameBuffer::bind()
{
    glViewport(0, 0, m_width, m_height);
    glBindFramebuffer(GL_FRAMEBUFFER, m_handler);

    return shared_from_this();
}

std::shared_ptr<SGCore::IFrameBuffer> SGCore::GL4FrameBuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    int wndWidth;
    int wndHeight;
    CoreMain::getWindow().getSize(wndWidth, wndHeight);
    glViewport(0, 0, wndWidth, wndHeight);

    return shared_from_this();
}

std::shared_ptr<SGCore::IFrameBuffer> SGCore::GL4FrameBuffer::create()
{
    static bool staticInit = []() {
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
        SGCF_INFO("OpenGL max color attachments: " + std::to_string(maxColorAttachments), SG_LOG_GAPI_FILE);
        return true;
    }();

    glGenFramebuffers(1, &m_handler);

    glBindFramebuffer(GL_FRAMEBUFFER, m_handler);

    return shared_from_this();
}

void SGCore::GL4FrameBuffer::destroy()
{
    glDeleteFramebuffers(1, &m_handler);
}

std::shared_ptr<SGCore::IFrameBuffer> SGCore::GL4FrameBuffer::clear()
{
    glClearColor(m_bgColor.r, m_bgColor.g, m_bgColor.b, m_bgColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    return shared_from_this();
}

std::shared_ptr<SGCore::IFrameBuffer> SGCore::GL4FrameBuffer::addAttachment(SGFrameBufferAttachmentType attachmentType,
                                                                            SGGColorFormat format,
                                                                            SGGColorInternalFormat internalFormat,
                                                                            const int& mipLevel,
                                                                            const int& layer,
                                                                            bool useMultisampling,
                                                                            std::uint8_t multisamplingSamplesCount)
{
    if(m_attachments.find(attachmentType) != m_attachments.end())
    {
        SGCF_ERROR("Error when adding an attachment to the framebuffer: "
                   "an attachment with this type already exists.", SG_LOG_GAPI_FILE);

        return shared_from_this();
    }

    auto& newAttachment = m_attachments[attachmentType];

    newAttachment.m_format = format;
    newAttachment.m_internalFormat = internalFormat;
    newAttachment.m_mipLevel = mipLevel;
    newAttachment.m_layer = layer;
    newAttachment.m_useMultisampling = useMultisampling;
    newAttachment.m_multisamplingSamplesCount = multisamplingSamplesCount;

    // TODO: MAKE VERIFY INTERNAL FORMAT
    // TODO: MAKE VERIFY SIZE TYPE

    if(attachmentType >= SGFrameBufferAttachmentType::SGG_DEPTH_ATTACHMENT0 &&
       attachmentType <= SGFrameBufferAttachmentType::SGG_DEPTH_ATTACHMENT9)
    {
        glGenTextures(1, &newAttachment.m_handler);
        glBindTexture(GL_TEXTURE_2D, newAttachment.m_handler);

        glTexImage2D(GL_TEXTURE_2D,
                     mipLevel,
                     GLGraphicsTypesCaster::sggInternalFormatToGL(internalFormat),
                     m_width, m_height,
                     0,
                     GLGraphicsTypesCaster::sggFormatToGL(format),
                // todo: make customizable
                     GL_FLOAT,
                     nullptr
        );

        // TODO: make it customizable
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        // todo: make it customizable
        float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, newAttachment.m_handler, mipLevel
        );
    }
    else if(attachmentType >= SGFrameBufferAttachmentType::SGG_DEPTH_STENCIL_ATTACHMENT0 &&
            attachmentType <= SGFrameBufferAttachmentType::SGG_DEPTH_STENCIL_ATTACHMENT9)
    {
        glGenTextures(1, &newAttachment.m_handler);
        glBindTexture(GL_TEXTURE_2D, newAttachment.m_handler);

        glTexImage2D(GL_TEXTURE_2D,
                     mipLevel,
                     GLGraphicsTypesCaster::sggInternalFormatToGL(internalFormat),
                     m_width, m_height,
                     0,
                     GLGraphicsTypesCaster::sggFormatToGL(format),
                // todo: make customizable
                     GL_UNSIGNED_INT_24_8,
                     nullptr
        );

        // TODO: make it customizable
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        // todo: make it customizable
        //float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
        //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_DEPTH_STENCIL_ATTACHMENT,
                               GL_TEXTURE_2D, newAttachment.m_handler, mipLevel);
    }
    else if(attachmentType >= SGFrameBufferAttachmentType::SGG_COLOR_ATTACHMENT0 &&
            attachmentType <= SGFrameBufferAttachmentType::SGG_COLOR_ATTACHMENT31)
    {
        GLenum glType = !useMultisampling ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE;

        glGenTextures(1, &newAttachment.m_handler);
        glBindTexture(glType, newAttachment.m_handler);

        if(!useMultisampling)
        {
            glTexImage2D(glType,
                         mipLevel,
                         GLGraphicsTypesCaster::sggInternalFormatToGL(internalFormat),
                         m_width, m_height,
                         0,
                         GLGraphicsTypesCaster::sggFormatToGL(format),
                         GL_FLOAT,
                         nullptr
            );
        }
        else
        {
            glTexImage2DMultisample(glType,
                                    multisamplingSamplesCount,
                                    GLGraphicsTypesCaster::sggFormatToGL(format),
                                    m_width, m_height,
                                    GL_FALSE
            );
        }

        // TODO: make it customizable
        glTexParameteri(glType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(glType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0 + (attachmentType - SGFrameBufferAttachmentType::SGG_COLOR_ATTACHMENT0),
                               glType, newAttachment.m_handler, mipLevel);
    }
    else if(attachmentType >= SGFrameBufferAttachmentType::SGG_RENDER_ATTACHMENT0 &&
            attachmentType <= SGFrameBufferAttachmentType::SGG_RENDER_ATTACHMENT9)
    {
        // todo: make
    }

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        SGCF_ERROR("Error when adding attachment to framebuffer: framebuffer is not completed", SG_LOG_GAPI_FILE);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    return shared_from_this();
}

std::shared_ptr<SGCore::IFrameBuffer>
SGCore::GL4FrameBuffer::addAttachment(SGFrameBufferAttachmentType attachmentType,
                                      SGGColorFormat format,
                                      SGGColorInternalFormat internalFormat,
                                      const int& mipLevel,
                                      const int& layer)
{
    return addAttachment(attachmentType, format, internalFormat, mipLevel, layer, false, 8);
}

void SGCore::GL4FrameBuffer::getAttachmentsCount(uint16_t& depthAttachmentsCount,
                                                         uint16_t& depthStencilAttachmentsCount,
                                                         uint16_t& colorAttachmentsCount,
                                                         uint16_t& renderAttachmentsCount) const noexcept
{
    depthAttachmentsCount = 0;
    depthStencilAttachmentsCount = 0;
    colorAttachmentsCount = 0;
    renderAttachmentsCount = 0;

    for(const auto& attachment : m_attachments)
    {
        if(attachment.first >= SGFrameBufferAttachmentType::SGG_DEPTH_ATTACHMENT0 &&
           attachment.first <= SGFrameBufferAttachmentType::SGG_DEPTH_ATTACHMENT9)
        {
            ++depthAttachmentsCount;
        }
        else if(attachment.first >= SGFrameBufferAttachmentType::SGG_DEPTH_STENCIL_ATTACHMENT0 &&
                attachment.first <= SGFrameBufferAttachmentType::SGG_DEPTH_STENCIL_ATTACHMENT9)
        {
            ++depthStencilAttachmentsCount;
        }
        else if(attachment.first >= SGFrameBufferAttachmentType::SGG_COLOR_ATTACHMENT0 &&
                attachment.first <= SGFrameBufferAttachmentType::SGG_COLOR_ATTACHMENT31)
        {
            ++colorAttachmentsCount;
        }
        else if(attachment.first >= SGFrameBufferAttachmentType::SGG_RENDER_ATTACHMENT0 &&
                attachment.first <= SGFrameBufferAttachmentType::SGG_RENDER_ATTACHMENT9)
        {
            ++renderAttachmentsCount;
        }
    }
}
