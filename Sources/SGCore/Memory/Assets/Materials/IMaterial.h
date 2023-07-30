#pragma once

#ifndef SUNGEARENGINE_IMATERIAL_H
#define SUNGEARENGINE_IMATERIAL_H

#include <map>
#include <string>

#include "SGCore/Graphics/API/ITexture2D.h"
#include "SGCore/Graphics/API/IShader.h"
#include "SGCore/Memory/AssetManager.h"
#include "SGCore/Memory/Assets/IAsset.h"

namespace Core::Graphics
{
    class IShader;
}

namespace Core::Memory::Assets
{
    struct MaterialTexture
    {
        std::string m_nameInShader;
        std::uint8_t m_textureUnit = 0;
        std::shared_ptr<Texture2DAsset> m_textureAsset;
    };

    struct MaterialTexturesBlock
    {
        // The maximum number of textures of this type
        std::uint8_t m_maximumTextures = 1;
        // Offset for texture block (beginning)
        std::uint8_t m_texturesUnitOffset = 0;
        // Textures of this type. Each of them also contains a path. It is not possible to add two textures with the same path
        std::map<std::string, MaterialTexture> m_textures;
    };

    class IMaterial : public std::enable_shared_from_this<IMaterial>, public IAsset
    {
    protected:
        // Blocks of textures that correspond to a specific type of texture
        std::map<SGMaterialTextureType, MaterialTexturesBlock> m_blocks;

        // Textures that could not be added to the material. This collection is used to migrate textures to another material
        std::map<SGMaterialTextureType, std::list<std::shared_ptr<Texture2DAsset>>> m_notUsedTextures;

        std::uint8_t m_maxUnit = 0;

    public:
        std::string m_name;

        std::shared_ptr<Graphics::IShader> m_shader;

        std::shared_ptr<IMaterial> bind();

        // TODO: impl
        std::shared_ptr<IAsset> load(const std::string& path) override;

        /**
        * Adds texture2D. Method is not copying texture.
        * @param type - Material type of texture
        * @param texture2DAsset - Texture asset
        * @return this
        */
        std::shared_ptr<IMaterial> addTexture2D(const SGMaterialTextureType& type,
                                                const std::shared_ptr<Memory::Assets::Texture2DAsset>& texture2DAsset);

        /**
        * Adds texture2D. Method is copying texture. This method is looking for texture asset by path.
        * @param type - Material type of texture
        * @param texture2DAsset - Texture asset
        * @return this
        */
        std::shared_ptr<Texture2DAsset> findAndAddTexture2D(const SGMaterialTextureType& type, const std::string& path);

        /**
         * Sets texture2D by name. Method is copying texture.
         * @param type - Material type of texture
         * @param oldTextureAssetPath - The path to the texture to be replaced
         * @param texture2DAsset - Texture asset that will replace the old texture
         *
         * @return this (shared_ptr)
         */
        std::shared_ptr<IMaterial> setTexture2D(const SGMaterialTextureType& type, const std::string_view& oldTextureAssetPath,
                                                const std::shared_ptr<Memory::Assets::Texture2DAsset>& texture2DAsset);
        /**
         * Sets texture2D by name. Method is copying texture. This method is looking for texture asset by path.
         * @param type - Material type of texture
         * @param oldTextureAssetPath - The path to the texture to be replaced
         * @param newTextureAssetPath - The path to the new texture
         *
         * @return this (shared_ptr)
         */
        std::shared_ptr<IMaterial> findAndSetTexture2D(const SGMaterialTextureType& type, const std::string& oldTextureAssetPath, const std::string& newTextureAssetPath);

        auto&& getTextures()
        {
            return m_blocks;
        }

        /**
         * Copies all texture assets (textures data is not copied) to another material.\n
         * Also tries to do the same with unused textures of this material.
         * @param other - Copied material
         * @return Modified this material
         */
        IMaterial& operator=(const IMaterial& other) noexcept;
    };
}

#endif //SUNGEARENGINE_IMATERIAL_H