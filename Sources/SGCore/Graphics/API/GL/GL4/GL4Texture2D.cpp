//
// Created by stuka on 28.05.2023.
//

#include "GL4Texture2D.h"
#include "SGCore/Graphics/API/GL/GL4/GL4Renderer.h"
#include "../GLGraphicsTypesCaster.h"

#include "SGCore/Logging/Log.h"

Core::Graphics::GL4Texture2D::~GL4Texture2D() noexcept
{
    destroy();
}

// migrate to gl46
void Core::Graphics::GL4Texture2D::create(std::weak_ptr<Memory::Assets::Texture2DAsset> asset) noexcept
{
    std::shared_ptr<Core::Memory::Assets::Texture2DAsset> originalSharedPtr = m_texture2DAsset.lock();

    if(originalSharedPtr && m_texture2DAsset.expired())
    {
        originalSharedPtr->removeObserver(this);
    }

    m_texture2DAsset = asset;

    originalSharedPtr = m_texture2DAsset.lock();

    originalSharedPtr->addObserver(this);

    m_glInternalFormat = GLGraphicsTypesCaster::sggInternalFormatToGL(originalSharedPtr->getInternalFormat());
    m_glFormat = GLGraphicsTypesCaster::sggFormatToGL(originalSharedPtr->getFormat());

    glGenTextures(1, &m_handler);
    glBindTexture(GL_TEXTURE_2D, m_handler);
    //glCreateTextures(GL_TEXTURE_2D, 1, &m_handler);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(m_handler, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(m_handler, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(m_handler, GL_GENERATE_MIPMAP, GL_TRUE);

    glTexParameteri(m_handler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(m_handler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /*glTextureStorage2D(m_handler, 1, m_glInternalFormat, originalSharedPtr->getWidth(), originalSharedPtr->getHeight());
    glTextureSubImage2D(m_handler,
                        0,
                        0,
                        0,
                        originalSharedPtr->getWidth(),
                        originalSharedPtr->getHeight(),
                        m_glFormat,
                        GL_UNSIGNED_BYTE,
                        reinterpret_cast<const void*>(originalSharedPtr->getData().get()));*/
    glTexStorage2D(GL_TEXTURE_2D, 1, m_glInternalFormat,
                   originalSharedPtr->getWidth(), originalSharedPtr->getHeight());

    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    originalSharedPtr->getWidth(),
                    originalSharedPtr->getHeight(),
                    m_glFormat,
                    GL_UNSIGNED_BYTE,
                    reinterpret_cast<const void*>(originalSharedPtr->getData().get()));


    #ifdef SUNGEAR_DEBUG
    GL4Renderer::getInstance()->checkForErrors();
    #endif
}

void Core::Graphics::GL4Texture2D::destroy() noexcept
{
    glDeleteTextures(1, &m_handler);
}

void Core::Graphics::GL4Texture2D::bind(const std::uint8_t& textureUnit) noexcept
{
    //glBindTextureUnit(textureUnit, m_handler);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, m_handler);
}

void Core::Graphics::GL4Texture2D::onAssetModified() noexcept
{

}

void Core::Graphics::GL4Texture2D::onAssetPathChanged() noexcept
{

}

void Core::Graphics::GL4Texture2D::onAssetDeleted() noexcept
{

}

void Core::Graphics::GL4Texture2D::onAssetRestored() noexcept
{

}

Core::Graphics::GL4Texture2D& Core::Graphics::GL4Texture2D::operator=
        (const std::shared_ptr<ITexture2D>& other)
{
    create(other->getAsset());

    return *this;
}