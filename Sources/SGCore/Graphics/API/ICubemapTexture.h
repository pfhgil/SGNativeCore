//
// Created by stuka on 29.08.2023.
//

#ifndef SUNGEARENGINE_ICUBEMAPTEXTURE_H
#define SUNGEARENGINE_ICUBEMAPTEXTURE_H

#include "ITexture2D.h"

#include "SGCore/Main/CoreGlobals.h"

namespace SGCore
{
    struct ICubemapTexture : public ITexture2D
    {
        friend class AssetManager;

        std::vector<AssetRef<ITexture2D>> m_parts;

    private:
        template<typename... AssetCtorArgs>
        static Ref<ICubemapTexture> createRefInstance(AssetCtorArgs&&... assetCtorArgs) noexcept
        {
            auto tex = Ref<ICubemapTexture>(CoreMain::getRenderer()->createCubemapTexture(std::forward<AssetCtorArgs>(assetCtorArgs)...));

            tex->addToGlobalStorage();

            return tex;
        }
    };
}

#endif //SUNGEARENGINE_ICUBEMAPTEXTURE_H
