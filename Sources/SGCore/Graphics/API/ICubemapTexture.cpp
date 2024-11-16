//
// Created by stuka on 29.08.2023.
//

#include "ICubemapTexture.h"
#include "SGCore/Memory/AssetManager.h"

void SGCore::ICubemapTexture::resolveMemberAssetsReferences(SGCore::AssetManager* parentAssetManager) noexcept
{
    for(auto& part : m_parts)
    {
        parentAssetManager->resolveAssetReference(part);
    }
}
