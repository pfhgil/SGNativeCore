#include <spdlog/spdlog.h>
#include <stb_image.h>

#include "ITexture2D.h"
#include "SGCore/Graphics/GPUObjectsStorage.h"

#define DDSKTX_IMPLEMENT

#include "SGUtils/DDSLoader/dds-ktx.h"
#include "SGUtils/FileUtils.h"

void SGCore::STBITextureDataDeleter::operator()(void* data)
{
    stbi_image_free(data);
}

// ----------------------------------

void SGCore::ITexture2D::doLoad(const std::string& path)
{
    int channelsDesired = 0;
    
    m_path = path;
    
    if(!std::filesystem::exists(path))
    {
        spdlog::error("Error while loading texture: texture by path {0} does not exist.", path);
        return;
    }
    
    // m_texture2D = Ref<ITexture2D>(CoreMain::getRenderer().createTexture2D())->addToGlobalStorage();

    const auto& ext = m_path.extension();
    
    if(ext == ".png" || ext == ".jpg" || ext == ".jpeg")
    {
        m_textureData = Ref<std::uint8_t[]>(
                stbi_load(m_path.string().data(),
                          &m_width, &m_height,
                          &m_channelsCount, channelsDesired),
                STBITextureDataDeleter {});
        
        if(m_channelsCount == 4)
        {
            m_internalFormat = SGGColorInternalFormat::SGG_RGBA8;
            m_format = SGGColorFormat::SGG_RGBA;
        }
        else if(m_channelsCount == 3)
        {
            m_internalFormat = SGGColorInternalFormat::SGG_RGB8;
            m_format = SGGColorFormat::SGG_RGB;
        }
    }
}

void SGCore::ITexture2D::doLazyLoad()
{
    create();
    
    addToGlobalStorage();
    
    std::cout << fmt::format("Loaded texture (in lazy load). Width: {0}, height: {1}, MB size: {2}, channels: {3}, path: {4}",
                             m_width,
                             m_height,
                             m_width * m_height * m_channelsCount / 1024.0 / 1024.0,
                             m_channelsCount,
                             m_path.string()) << std::endl;
    
    spdlog::info("Loaded texture (in lazy load). Width: {0}, height: {1}, MB size: {2}, channels: {3}, path: {4}",
                 m_width,
                 m_height,
                 m_width * m_height * m_channelsCount / 1024.0 / 1024.0,
                 m_channelsCount,
                 m_path.string());
}

void SGCore::ITexture2D::addToGlobalStorage() noexcept
{
    GPUObjectsStorage::addTexture(shared_from_this());
}

SGCore::Ref<std::uint8_t[]> SGCore::ITexture2D::getData() noexcept
{
    return m_textureData;
}

SGCore::ITexture2D::~ITexture2D()
{

}
