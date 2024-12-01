//
// Created by stuka on 07.05.2023.
//

#ifndef SUNGEARENGINE_ASSETMANAGER_H
#define SUNGEARENGINE_ASSETMANAGER_H

#include <SGCore/pch.h>

#include <sgcore_export.h>

#include "Assets/IAsset.h"
#include "SGCore/Utils/Utils.h"
#include "SGCore/Threading/ThreadsPool.h"
#include "SGCore/Threading/ThreadsManager.h"
#include "SGCore/Logger/Logger.h"
#include "AssetsPackage.h"
#include "AssetRef.h"
#include "AssetWeakRef.h"
#include "SGCore/Utils/Event.h"

namespace SGCore
{
    enum AssetsLoadPolicy
    {
        SINGLE_THREADED,
        PARALLEL_THEN_LAZYLOAD,
        PARALLEL_NO_LAZYLOAD
    };

    class SGCORE_EXPORT AssetManager : public std::enable_shared_from_this<AssetManager>
    {
    public:
        sg_serde_as_friend()

        using assets_container_t = std::unordered_map<size_t, std::unordered_map<size_t, Ref<IAsset>>>;
        using assets_refs_container_t = std::unordered_map<size_t, std::unordered_map<size_t, AssetRef<IAsset>>>;

        static void init() noexcept;

        void addStandardAssets() noexcept;

        AssetsLoadPolicy m_defaultAssetsLoadPolicy = AssetsLoadPolicy::SINGLE_THREADED;

        /**
         * Use this function to resolve member asset reference.\n
         * If the asset reference resolves, the asset data will be automatically loaded.
         * @tparam AssetT type of asset reference to resolve.
         * @param assetRef asset reference that is needs to be resolved.
         * @param isAssetRefWasResolved is asset reference was resolved.
         */
        template<typename AssetT>
        static void resolveAssetReference(AssetManager* updatedAssetManager, AssetRef<AssetT>& assetRef) noexcept
        {
            const auto lockedDeserializedAssetManager = assetRef.m_deserializedParentAssetManager.lock();

            // assetRef.m_asset can be nullptr only when assetRef was deserialized earlier but was not resolved earlier
            AssetManager* usedAssetManager = lockedDeserializedAssetManager.get();
            if(assetRef)
            {
                usedAssetManager = assetRef->getParentAssetManager().get();
            }

            // if names of updated asset manager and asset manager that is used for current asset are not equals
            // and assetRef contains valid asset then we do not resolving reference
            // because assetRef was resolved earlier and its parent asset manager was not updated
            if(usedAssetManager->getName() != updatedAssetManager->getName() && assetRef) return;

            const std::string assetAlias = assetRef.m_asset ? assetRef->getAlias() : assetRef.m_deserializedAssetAlias;
            const InterpolatedPath assetPath = assetRef.m_asset ? assetRef->getPath() : assetRef.m_deserializedAssetPath;
            const AssetStorageType assetStoredBy = assetRef.m_asset ? assetRef->storedByWhat() : assetRef.m_deserializedAssetStoredBy;
            const size_t assetTypeID = assetRef.m_asset ? assetRef->getTypeID() : assetRef.m_deserializedAssetTypeID;

            auto newAssetRef = usedAssetManager->loadExistingAsset(assetAlias, assetPath, assetStoredBy, assetTypeID);

            if(!newAssetRef)
            {
                LOG_W(SGCORE_TAG, "Can not resolve asset reference: can not find asset in manager! Info about asset reference: alias - '{}', path - '{}', stored by - '{}', asset type ID - '{}'",
                      assetAlias,
                      Utils::toUTF8(assetPath.resolved().u16string()),
                      std::to_underlying(assetStoredBy),
                      assetTypeID);

                return;
            }

            assetRef = newAssetRef.template staticCast<AssetT>();

            LOG_I(SGCORE_TAG, "Asset reference was resolved! Info about asset reference: alias - '{}', path - '{}', stored by - '{}', asset type ID - '{}'",
                  assetAlias,
                  Utils::toUTF8(assetPath.resolved().u16string()),
                  std::to_underlying(assetStoredBy),
                  assetTypeID);
        }

        template<typename AssetT>
        static void resolveWeakAssetReference(AssetManager* updatedAssetManager, AssetWeakRef<AssetT>& assetRef) noexcept
        {
            auto strongRef = assetRef.lock();
            resolveAssetReference(updatedAssetManager, strongRef);

            assetRef = strongRef;
        }

        static Ref<AssetManager> getAssetManager(const std::string& name) noexcept
        {
            auto foundIt = s_allAssetsManagers.find(name);
            if(foundIt != s_allAssetsManagers.end())
            {
                return foundIt->second;
            }

            Ref<AssetManager> assetManager = Ref<AssetManager>(new AssetManager(name));
            s_allAssetsManagers[name] = assetManager;

            return assetManager;
        }

        template<typename AssetT, typename... AssetCtorArgsT>
        requires(std::is_base_of_v<IAsset, AssetT>)
        AssetRef<AssetT> createAsset(AssetCtorArgsT&&... assetCtorArgs) noexcept
        {
            Ref<AssetT> asset = createAssetInstance<AssetT>(std::forward<AssetCtorArgsT>(assetCtorArgs)...);
            m_assets[hashString(asset->getAlias())][AssetT::type_id] = asset;

            return AssetRef<AssetT>(asset);
        }

        /// CALL ONLY AFTER RESOLVING CONFLICTS OF ASSETS.\n
        /// CALL THIS FUNCTION IMMEDIATELY AFTER DESERIALIZATION OF ASSETS IF NO CONFLICTS WERE FOUND.\n
        void resolveMemberAssetsReferences() noexcept;

        /**
         * Subscriber of this event MUST call onAssetsReferencesResolve event after resolving conflicts.
         * First argument - deserialized assets from package that are conflicting with assets of current asset manager.
         */
        std::function<void(const assets_refs_container_t& deserializedConflictingAssets, AssetManager* assetManager)> deserializedAssetsConflictsResolver;

        // =================================================================================================
        // =================================================================================================
        // =================================================================================================
        
        template<typename AssetT, typename... AssetCtorArgsT>
        requires(std::is_base_of_v<IAsset, AssetT>)
        AssetRef<AssetT> loadAsset(AssetsLoadPolicy assetsLoadPolicy,
                                   const Ref<Threading::Thread>& lazyLoadInThread,
                                   const InterpolatedPath& path,
                                   AssetCtorArgsT&& ... assetCtorArgs)
        {
            const size_t hashedAssetPath = hashString(Utils::toUTF8(path.resolved().u16string()));

            std::unordered_map<size_t, Ref<IAsset>>* foundVariants { };

            {
                std::lock_guard guard(m_mutex);
                // getting variants of assets that were loaded by path 'path'
                foundVariants = &m_assets[hashedAssetPath];
            }

            // trying to find existing asset of type 'AssetT' loaded bt path 'path'
            auto foundAssetOfTIt = foundVariants->find(AssetT::type_id);
            // if asset already exists then we are just leaving
            if(foundAssetOfTIt != foundVariants->end())
            {
                auto asset = foundAssetOfTIt->second;

                // if asset was not loaded earlier
                // THIS IS DEFERRED LOAD OF ASSET (LOAD AS NEEDED)
                if(!asset->m_isLoaded)
                {
                    distributeAsset(asset, path, assetsLoadPolicy, lazyLoadInThread);
                    LOG_I(SGCORE_TAG, "Loaded new asset associated by path: {}. Asset type: {}. Asset type ID: {}",
                          Utils::toUTF8(path.resolved().u16string()), typeid(AssetT).name(), AssetT::type_id);
                }

                // WE ARE USING STATIC CAST BECAUSE WE KNOW THAT ONLY AN ASSET WITH THE ASSET TYPE HAS SUCH AN asset_type_id.
                // IF THIS IS NOT THE CASE, AND SUDDENLY IT TURNS OUT THAT THERE ARE TWO OR MORE ASSET CLASSES WITH THE SAME asset_type_id, THEN ALAS (i hope engine will crash).
                return AssetRef<AssetT>(std::static_pointer_cast<AssetT>(foundAssetOfTIt->second));
            }

            Ref<AssetT> newAsset = createAssetInstance<AssetT>(std::forward<AssetCtorArgsT>(assetCtorArgs)...);
            newAsset->m_parentAssetManager = shared_from_this();

            {
                std::lock_guard guard(m_mutex);
                (*foundVariants)[AssetT::type_id] = newAsset;
            }

            newAsset->m_path = path;
            newAsset->m_storedBy = AssetStorageType::BY_PATH;
            
            distributeAsset(newAsset, path, assetsLoadPolicy, lazyLoadInThread);

            LOG_I(SGCORE_TAG, "Loaded new asset associated by path: {}. Asset type: {}. Asset type ID: {}",
                  Utils::toUTF8(path.resolved().u16string()), typeid(AssetT).name(), AssetT::type_id);
            
            return AssetRef<AssetT>(newAsset);
        }
        
        template<typename AssetT, typename... AssetCtorArgs>
        requires(std::is_base_of_v<IAsset, AssetT>)
        AssetRef<AssetT> loadAsset(AssetsLoadPolicy assetsLoadPolicy,
                                   const InterpolatedPath& path,
                                   AssetCtorArgs&& ... assetCtorArgs)
        {
            return loadAsset<AssetT, AssetCtorArgs...>(assetsLoadPolicy, Threading::ThreadsManager::getMainThread(), path, std::forward<AssetCtorArgs>(assetCtorArgs)...);
        }
        
        template<typename AssetT, typename... AssetCtorArgs>
        requires(std::is_base_of_v<IAsset, AssetT>)
        AssetRef<AssetT> loadAsset(const InterpolatedPath& path,
                                   AssetCtorArgs&& ... assetCtorArgs)
        {
            return loadAsset<AssetT, AssetCtorArgs...>(m_defaultAssetsLoadPolicy, Threading::ThreadsManager::getMainThread(), path, std::forward<AssetCtorArgs>(assetCtorArgs)...);
        }
        
        // =================================================================================================
        // =================================================================================================
        // =================================================================================================
        
        template<typename AssetT>
        requires(std::is_base_of_v<IAsset, AssetT>)
        void loadAsset(AssetRef<AssetT>& assetToLoad,
                       AssetsLoadPolicy assetsLoadPolicy,
                       const Ref<Threading::Thread>& lazyLoadInThread,
                       const InterpolatedPath& path)
        {
            std::lock_guard guard(m_mutex);

            const size_t hashedAssetPath = hashString(Utils::toUTF8(path.resolved().u16string()));

            // getting variants of assets that were loaded by path 'path'
            auto& foundVariants = m_assets[hashedAssetPath];

            // trying to find existing asset of type 'AssetT' loaded bt path 'path'
            auto foundAssetOfTIt = foundVariants.find(AssetT::type_id);
            // if asset already exists then we are just leaving
            if(foundAssetOfTIt != foundVariants.end())
            {
                // if asset was not loaded earlier
                // THIS IS DEFERRED LOAD OF ASSET (LOAD AS NEEDED)
                if(!assetToLoad->m_isLoaded)
                {
                    distributeAsset(assetToLoad.m_asset, path, assetsLoadPolicy, lazyLoadInThread);
                    LOG_I(SGCORE_TAG, "Loaded new asset associated by path: {}. Asset type: {}",
                          Utils::toUTF8(path.resolved().u16string()), typeid(AssetT).name());
                }

                return;
            }

            foundVariants[AssetT::type_id] = assetToLoad.m_asset;
            if(assetToLoad->getParentAssetManager())
            {
                assetToLoad->getParentAssetManager()->removeAsset(assetToLoad);
            }
            assetToLoad->m_parentAssetManager = shared_from_this();

            assetToLoad->m_path = path;
            assetToLoad->m_storedBy = AssetStorageType::BY_PATH;
            
            distributeAsset(assetToLoad.m_asset, path, assetsLoadPolicy, lazyLoadInThread);

            LOG_I(SGCORE_TAG, "Loaded new asset associated by path: {}. Asset type: {}",
                  Utils::toUTF8(path.resolved().u16string()), typeid(AssetT).name());
        }
        
        template<typename AssetT>
        requires(std::is_base_of_v<IAsset, AssetT>)
        void loadAsset(AssetRef<AssetT>& assetToLoad,
                       AssetsLoadPolicy assetsLoadPolicy,
                       const InterpolatedPath& path)
        {
            loadAsset<AssetT>(assetToLoad, assetsLoadPolicy, Threading::ThreadsManager::getMainThread(), path);
        }
        
        template<typename AssetT>
        requires(std::is_base_of_v<IAsset, AssetT>)
        void loadAsset(AssetRef<AssetT>& assetToLoad,
                       const InterpolatedPath& path)
        {
            loadAsset<AssetT>(assetToLoad, m_defaultAssetsLoadPolicy, Threading::ThreadsManager::getMainThread(), path);
        }
        
        // =================================================================================================
        // =================================================================================================
        // =================================================================================================

        /**
         * Use for asset deferred loading. Returns nullptr if asset with this alias or path does not exist.\n
         * Loads asset if \p m_isLoaded variable of asset equals to false.
         * @tparam AssetCtorArgsT
         * @param alias
         * @param path
         * @param loadedBy
         * @param assetTypeID
         * @param assetsLoadPolicy
         * @param lazyLoadInThread
         * @param assetCtorArgs
         * @return
         */
        AssetRef<IAsset> loadExistingAsset(const std::string& alias,
                                           const InterpolatedPath& path,
                                           AssetStorageType loadedBy,
                                           const size_t& assetTypeID,
                                           AssetsLoadPolicy assetsLoadPolicy,
                                           const Ref<Threading::Thread>& lazyLoadInThread) noexcept
        {
            size_t hashedAssetPath { };
            switch(loadedBy)
            {
                case AssetStorageType::BY_PATH:
                    hashedAssetPath = hashString(Utils::toUTF8(path.resolved().u16string()));
                    break;
                case AssetStorageType::BY_ALIAS:
                    hashedAssetPath = hashString(alias);
                    break;
            }

            // getting variants of assets that were loaded by path 'path'
            std::unordered_map<size_t, Ref<IAsset>>* foundVariants { };
            {
                std::lock_guard guard(m_mutex);
                foundVariants = &m_assets[hashedAssetPath];
            }

            // trying to find existing asset of type 'AssetT' loaded bt path 'path'
            auto foundAssetOfTIt = foundVariants->find(assetTypeID);
            // if asset already exists then we are just leaving
            if(foundAssetOfTIt != foundVariants->end())
            {
                auto asset = foundAssetOfTIt->second;

                // if asset was not loaded earlier
                // THIS IS DEFERRED LOAD OF ASSET (LOAD AS NEEDED)
                if(!asset->m_isLoaded)
                {
                    distributeAsset(asset, path, assetsLoadPolicy, lazyLoadInThread);
                    LOG_I(SGCORE_TAG, "Loaded existing asset (deferred load) with path: {}; and alias: {}. Asset type ID: {}",
                          Utils::toUTF8(path.resolved().u16string()), alias, assetTypeID);
                }

                return AssetRef<IAsset>(foundAssetOfTIt->second);
            }

            return nullptr;
        }

        AssetRef<IAsset> loadExistingAsset(const std::string& alias,
                                           const InterpolatedPath& path,
                                           AssetStorageType loadedBy,
                                           const size_t& assetTypeID,
                                           AssetsLoadPolicy assetsLoadPolicy) noexcept
        {
            return loadExistingAsset(alias, path, loadedBy, assetTypeID, assetsLoadPolicy,
                                     Threading::ThreadsManager::getMainThread());
        }

        AssetRef<IAsset> loadExistingAsset(const std::string& alias,
                                           const InterpolatedPath& path,
                                           AssetStorageType loadedBy,
                                           const size_t& assetTypeID) noexcept
        {
            return loadExistingAsset(alias, path, loadedBy, assetTypeID, m_defaultAssetsLoadPolicy,
                                     Threading::ThreadsManager::getMainThread());
        }

        // =================================================================================================
        // =================================================================================================
        // =================================================================================================
        
        template<typename AssetT>
        requires(std::is_base_of_v<IAsset, AssetT>)
        void loadAssetWithAlias(AssetRef<AssetT>& assetToLoad,
                                AssetsLoadPolicy assetsLoadPolicy,
                                const Ref<Threading::Thread>& lazyLoadInThread,
                                const std::string& alias,
                                const InterpolatedPath& path)
        {
            std::lock_guard guard(m_mutex);

            const size_t hashedAssetAlias = hashString(alias);

            // getting variants of assets that were loaded with alias 'alias'
            auto& foundVariants = m_assets[hashedAssetAlias];

            // trying to find existing asset of type 'AssetT' loaded with alias 'alias'
            auto foundAssetOfTIt = foundVariants.find(AssetT::type_id);
            // if asset already exists then we are just leaving
            if(foundAssetOfTIt != foundVariants.end())
            {
                // if asset was not loaded earlier
                // THIS IS DEFERRED LOAD OF ASSET (LOAD AS NEEDED)
                if(!assetToLoad->m_isLoaded)
                {
                    distributeAsset(assetToLoad.m_asset, path, assetsLoadPolicy, lazyLoadInThread);
                    LOG_I(SGCORE_TAG, "Loaded new asset associated by path: {}. Asset type: {}",
                          Utils::toUTF8(path.resolved().u16string()), typeid(AssetT).name());
                }

                return;
            }

            // else we are assigning asset of type 'AssetT'
            foundVariants[AssetT::type_id] = assetToLoad.m_asset;
            if(assetToLoad->getParentAssetManager())
            {
                assetToLoad->getParentAssetManager()->removeAsset(assetToLoad);
            }
            assetToLoad->m_parentAssetManager = shared_from_this();

            assetToLoad->m_alias = alias;
            assetToLoad->m_storedBy = AssetStorageType::BY_ALIAS;
            
            distributeAsset(assetToLoad.m_asset, path, assetsLoadPolicy, lazyLoadInThread);

            LOG_I(SGCORE_TAG, "Loaded new asset associated by path: {}. Asset type: {}",
                  Utils::toUTF8(path.resolved().u16string()), typeid(AssetT).name());
        }
        
        template<typename AssetT>
        requires(std::is_base_of_v<IAsset, AssetT>)
        void loadAssetWithAlias(AssetRef<AssetT>& assetToLoad,
                                AssetsLoadPolicy assetsLoadPolicy,
                                const std::string& alias,
                                const InterpolatedPath& path)
        {
            loadAssetWithAlias<AssetT>(assetToLoad, assetsLoadPolicy, Threading::ThreadsManager::getMainThread(), alias, path);
        }
        
        template<typename AssetT>
        requires(std::is_base_of_v<IAsset, AssetT>)
        void loadAssetWithAlias(AssetRef<AssetT>& assetToLoad,
                                const std::string& alias,
                                const InterpolatedPath& path)
        {
            loadAssetWithAlias<AssetT>(assetToLoad, m_defaultAssetsLoadPolicy, Threading::ThreadsManager::getMainThread(), alias, path);
        }
        
        // =================================================================================================
        // =================================================================================================
        // =================================================================================================
        
        template<typename AssetT, typename... AssetCtorArgsT>
        requires(std::is_base_of_v<IAsset, AssetT>)
        AssetRef<AssetT> loadAssetWithAlias(AssetsLoadPolicy assetsLoadPolicy,
                                            const Ref<Threading::Thread>& lazyLoadInThread,
                                            const std::string& alias,
                                            const InterpolatedPath& path,
                                            AssetCtorArgsT&& ... assetCtorArgs)
        {
            std::lock_guard guard(m_mutex);

            const size_t hashedAssetAlias = hashString(alias);

            // getting variants of assets that were loaded with alias 'alias'
            auto& foundVariants = m_assets[hashedAssetAlias];

            // trying to find existing asset of type 'AssetT' loaded with alias 'alias'
            auto foundAssetOfTIt = foundVariants.find(AssetT::type_id);
            // if asset already exists then we are just leaving
            if(foundAssetOfTIt != foundVariants.end())
            {
                auto asset = foundAssetOfTIt->second;

                // if asset was not loaded earlier
                // THIS IS DEFERRED LOAD OF ASSET (LOAD AS NEEDED)
                if(!asset->m_isLoaded)
                {
                    distributeAsset(asset, path, assetsLoadPolicy, lazyLoadInThread);
                    LOG_I(SGCORE_TAG, "Loaded new asset associated by path: {}; and alias: {}. Asset type: {}. Asset type ID: {}",
                          Utils::toUTF8(path.resolved().u16string()), alias, typeid(AssetT).name(), AssetT::type_id);
                }

                // WE ARE USING STATIC CAST BECAUSE WE KNOW THAT ONLY AN ASSET WITH THE ASSET TYPE HAS SUCH AN asset_type_id.
                // IF THIS IS NOT THE CASE, AND SUDDENLY IT TURNS OUT THAT THERE ARE TWO OR MORE ASSET CLASSES WITH THE SAME asset_type_id, THEN ALAS (i hope engine will crash).
                return AssetRef<AssetT>(std::static_pointer_cast<AssetT>(asset));
            }

            // else we are creating new asset with type 'AssetT'
            Ref<AssetT> newAsset = createAssetInstance<AssetT>(std::forward<AssetCtorArgsT>(assetCtorArgs)...);
            newAsset->m_parentAssetManager = shared_from_this();
            foundVariants[AssetT::type_id] = newAsset;

            newAsset->m_alias = alias;
            newAsset->m_storedBy = AssetStorageType::BY_ALIAS;

            distributeAsset(newAsset, path, assetsLoadPolicy, lazyLoadInThread);

            LOG_I(SGCORE_TAG, "Loaded new asset associated by path: {}; and alias: {}. Asset type: {}. Asset type ID: {}",
                  Utils::toUTF8(path.resolved().u16string()), alias, typeid(AssetT).name(), AssetT::type_id);
            
            return AssetRef<AssetT>(newAsset);
        }
        
        template<typename AssetT, typename... AssetCtorArgs>
        requires(std::is_base_of_v<IAsset, AssetT>)
        AssetRef<AssetT> loadAssetWithAlias(AssetsLoadPolicy assetsLoadPolicy,
                                            const std::string& alias,
                                            const InterpolatedPath& path,
                                            AssetCtorArgs&& ... assetCtorArgs)
        {
            return loadAssetWithAlias<AssetT, AssetCtorArgs...>(assetsLoadPolicy, Threading::ThreadsManager::getMainThread(), alias, path, std::forward<AssetCtorArgs>(assetCtorArgs)...);
        }
        
        template<typename AssetT, typename... AssetCtorArgs>
        requires(std::is_base_of_v<IAsset, AssetT>)
        AssetRef<AssetT> loadAssetWithAlias(const std::string& alias,
                                            const InterpolatedPath& path,
                                            AssetCtorArgs&& ... assetCtorArgs)
        {
            return loadAssetWithAlias<AssetT, AssetCtorArgs...>(m_defaultAssetsLoadPolicy, Threading::ThreadsManager::getMainThread(), alias, path, std::forward<AssetCtorArgs>(assetCtorArgs)...);
        }
        
        // =================================================================================================
        // =================================================================================================
        // =================================================================================================
        
        template<typename AssetT>
        requires(std::is_base_of_v<IAsset, AssetT>)
        void addAssetByAlias(const std::string& alias, const AssetRef<AssetT>& asset)
        {
            std::lock_guard guard(m_mutex);

            const size_t hashedAssetAlias = hashString(alias);

            asset->m_alias = alias;
            asset->m_storedBy = AssetStorageType::BY_ALIAS;

            // getting variants of assets that were loaded with alias 'alias'
            auto& foundVariants = m_assets[hashedAssetAlias];

            // trying to find existing asset of type 'AssetT' loaded with alias 'alias'
            auto foundAssetOfTIt = foundVariants.find(AssetT::type_id);
            // if asset already exists then we are just leaving
            if(foundAssetOfTIt != foundVariants.end())
            {
                return;
            }

            // assigning new asset by type 'AssetT'
            foundVariants[AssetT::type_id] = asset.m_asset;
            if(asset->getParentAssetManager())
            {
                asset->getParentAssetManager()->removeAsset(asset);
            }
            asset->m_parentAssetManager = shared_from_this();

            LOG_I(SGCORE_TAG, "Added new asset with alias '{}', path '{}' and type '{}'",
                  alias, Utils::toUTF8(asset->getPath().resolved().u16string()), typeid(AssetT).name())
        }
        
        template<typename AssetT>
        requires(std::is_base_of_v<IAsset, AssetT>)
        void addAssetByPath(const InterpolatedPath& assetPath, const AssetRef<AssetT>& asset)
        {
            std::lock_guard guard(m_mutex);

            const size_t hashedAssetPath = hashString(Utils::toUTF8(assetPath.resolved().u16string()));

            asset->m_path = assetPath;
            asset->m_storedBy = AssetStorageType::BY_PATH;

            // getting variants of assets that were loaded by path 'assetPath'
            auto& foundVariants = m_assets[hashedAssetPath];

            // trying to find existing asset of type 'AssetT' loaded by path 'assetPath'
            auto foundAssetOfTIt = foundVariants.find(AssetT::type_id);
            // if asset already exists then we are just leaving
            if(foundAssetOfTIt != foundVariants.end())
            {
                return;
            }

            // assigning new asset by type 'AssetT'
            foundVariants[AssetT::type_id] = asset;
            if(asset->getParentAssetManager())
            {
                asset->getParentAssetManager()->removeAsset(asset);
            }
            asset->m_parentAssetManager = shared_from_this();

            LOG_I(SGCORE_TAG, "Added new asset with alias '{}', path '{}' and type '{}'",
                  asset->m_alias, Utils::toUTF8(asset->getPath().resolved().u16string()), typeid(AssetT).name())
        }
        
        bool isAnyAssetLoadedByPathOrAlias(const std::string& pathOrAlias) noexcept
        {
            auto foundVariantsIt = m_assets.find(hashString(pathOrAlias));
            
            return foundVariantsIt != m_assets.end();
        }
        
        template<typename AssetT>
        requires(std::is_base_of_v<IAsset, AssetT>)
        bool isAssetExists(const std::string& pathOrAlias) noexcept
        {
            std::lock_guard guard(m_mutex);
            
            auto foundVariantsIt = m_assets.find(hashString(pathOrAlias));
            if(foundVariantsIt == m_assets.end())
            {
                return false;
            }
            else
            {
                auto foundAssetIt = foundVariantsIt->second.find(AssetT::type_id);
                return foundAssetIt != foundVariantsIt->second.end();
            }
        }

        template<typename AssetT>
        requires(std::is_base_of_v<IAsset, AssetT>)
        bool isAssetExists(const AssetT* asset) noexcept
        {
            std::lock_guard guard(m_mutex);

            std::string pathOrAlias;
            switch(asset->m_storedBy)
            {
                case AssetStorageType::BY_PATH:
                {
                    pathOrAlias = Utils::toUTF8(asset->m_path.resolved().u16string());
                    break;
                }
                case AssetStorageType::BY_ALIAS:
                {
                    pathOrAlias = asset->m_alias;
                    break;
                }
            }

            auto foundVariantsIt = m_assets.find(hashString(pathOrAlias));
            if(foundVariantsIt == m_assets.end())
            {
                return false;
            }
            else
            {
                auto foundAssetIt = foundVariantsIt->second.find(asset->getTypeID());
                return foundAssetIt != foundVariantsIt->second.end();
            }
        }

        bool isAssetExists(const std::string& pathOrAlias, const size_t& assetTypeID) noexcept;
        bool isAssetExists(const std::string& alias, const InterpolatedPath& path, AssetStorageType loadedBy,
                           const size_t& assetTypeID) noexcept;

        /**
         * Completely deletes the asset (deletes all copies of the asset that were loaded as different types).
         * @param aliasOrPath
         */
        void fullRemoveAsset(const InterpolatedPath& aliasOrPath) noexcept;

        /**
         * Deletes only one copy of an asset that was loaded as type AssetT.
         * @tparam AssetT
         * @param aliasOrPath
         */
        template<typename AssetT>
        requires(std::is_base_of_v<IAsset, AssetT>)
        void removeAssetLoadedByType(const InterpolatedPath& aliasOrPath) noexcept
        {
            auto foundIt = m_assets.find(hashString(Utils::toUTF8(aliasOrPath.resolved().u16string())));
            if(foundIt == m_assets.end()) return;

            auto& variants = foundIt->second;
            variants.erase(AssetT::type_id);
        }

        /**
         * Deletes only one copy of an asset that was loaded as type AssetT.
         * @tparam AssetT
         * @param aliasOrPath
         */
        template<typename AssetT>
        requires(std::is_base_of_v<IAsset, AssetT>)
        void removeAsset(const AssetRef<AssetT>& asset) noexcept
        {
            size_t pathOrAliasHash;
            switch(asset->m_storedBy)
            {
                case AssetStorageType::BY_PATH:
                    pathOrAliasHash = hashString(Utils::toUTF8(asset->getPath().resolved().u16string()));
                    break;

                case AssetStorageType::BY_ALIAS:
                    pathOrAliasHash = hashString(asset->getAlias());
                    break;
            }

            auto foundIt = m_assets.find(pathOrAliasHash);
            if(foundIt == m_assets.end()) return;

            auto& variants = foundIt->second;
            variants.erase(AssetT::type_id);
        }

        template<typename AssetT>
        requires(std::is_base_of_v<IAsset, AssetT>)
        [[nodiscard]] std::vector<AssetRef<AssetT>> getAssetsWithType() const noexcept
        {
            std::vector<AssetRef<AssetT>> assets;
            for(const auto& p0 : m_assets)
            {
                auto foundAssetWithTIt = p0.second.find(AssetT::type_id);
                if(foundAssetWithTIt == p0.second.end())
                {
                    continue;
                }

                // WE ARE USING STATIC CAST BECAUSE WE KNOW THAT ONLY AN ASSET WITH THE ASSET TYPE HAS SUCH AN asset_type_id.
                // IF THIS IS NOT THE CASE, AND SUDDENLY IT TURNS OUT THAT THERE ARE TWO OR MORE ASSET CLASSES WITH THE SAME asset_type_id, THEN ALAS (i hope engine will crash).
                assets.push_back(AssetRef<AssetT>(std::static_pointer_cast<AssetT>(foundAssetWithTIt->second)));
            }

            return assets;
        }

        /**
         * Gets asset in this asset manager using data from \p byAsset .
         * @param byAsset
         * @return
         */
        template<typename AssetT>
        [[nodiscard]] AssetRef<AssetT> getAsset(const AssetRef<AssetT>& byAsset) noexcept
        {
            size_t pathOrAliasHash { };
            switch(byAsset->m_storedBy)
            {
                case AssetStorageType::BY_PATH:
                    pathOrAliasHash = hashString(Utils::toUTF8(byAsset->getPath().resolved().u16string()));
                    break;
                case AssetStorageType::BY_ALIAS:
                    pathOrAliasHash = hashString(byAsset->getAlias());
                    break;
            }

            if(!m_assets.contains(pathOrAliasHash)) return nullptr;

            if(!m_assets[pathOrAliasHash][byAsset->getTypeID()]) return nullptr;

            return AssetRef<AssetT>(std::static_pointer_cast<AssetT>(m_assets[pathOrAliasHash][byAsset->getTypeID()]));
        }

        template<typename AssetT>
        requires(std::is_base_of_v<IAsset, AssetT>)
        [[nodiscard]] AssetRef<AssetT> getOrAddAssetByAlias(const std::string& alias) noexcept
        {
            const size_t aliasHash = hashString(alias);

            auto foundVariantsIt = m_assets.find(aliasHash);

            if(foundVariantsIt == m_assets.end())
            {
                return createAssetWithAlias<AssetT>(alias);
            }

            auto foundAssetIt = foundVariantsIt->second.find(AssetT::type_id);

            if(foundAssetIt == foundVariantsIt->second.end())
            {
                return createAssetWithAlias<AssetT>(alias);
            }

            return AssetRef<AssetT>(std::static_pointer_cast<AssetT>(foundAssetIt->second));
        }

        template<typename AssetT>
        requires(std::is_base_of_v<IAsset, AssetT>)
        [[nodiscard]] AssetRef<AssetT> getOrAddAssetByPath(const InterpolatedPath& path) noexcept
        {
            const size_t pathHash = hashString(Utils::toUTF8(path.resolved().u16string()));

            auto foundVariantsIt = m_assets.find(pathHash);

            if(foundVariantsIt == m_assets.end())
            {
                return createAssetWithPath<AssetT>(path);
            }

            auto foundAssetIt = foundVariantsIt->second.find(AssetT::type_id);

            if(foundAssetIt == foundVariantsIt->second.end())
            {
                return createAssetWithPath<AssetT>(path);
            }

            return AssetRef<AssetT>(std::static_pointer_cast<AssetT>(foundAssetIt->second));
        }

        template<typename AssetT>
        requires(std::is_base_of_v<IAsset, AssetT>)
        [[nodiscard]] AssetRef<AssetT> getAsset(const std::string& pathOrAlias) noexcept
        {
            const size_t pathOrAliasHash = hashString(pathOrAlias);

            auto foundVariantsIt = m_assets.find(pathOrAliasHash);

            if(foundVariantsIt == m_assets.end())
            {
                return nullptr;
            }

            auto foundAssetIt = foundVariantsIt->second.find(AssetT::type_id);

            if(foundAssetIt == foundVariantsIt->second.end())
            {
                return nullptr;
            }

            return AssetRef<AssetT>(std::static_pointer_cast<AssetT>(foundAssetIt->second));
        }

        [[nodiscard]] AssetRef<IAsset> getAsset(const std::string& pathOrAlias, const size_t& assetTypeID) noexcept;

        void clear() noexcept;

        [[nodiscard]] SG_NOINLINE static Ref<AssetManager> getInstance() noexcept;

        void createPackage(const InterpolatedPath& toDirectory, const std::string& packageName) noexcept;
        void loadPackage(const InterpolatedPath& fromDirectory, const std::string& packageName) noexcept;

        [[nodiscard]] const AssetsPackage& getPackage() const noexcept;

        const std::string& getName() const noexcept;

        /// This event is using for resolve references of member assets after package deserialization.
        /// This event is called after package deserialization.
        /// You can subscribe to this event to resolve member assets references.
        [[nodiscard]] SG_NOINLINE static auto& getOnMemberAssetsReferencesResolveEvent() noexcept
        {
            return onMemberAssetsReferencesResolve;
        }

    private:
        explicit AssetManager(const std::string& name) noexcept : m_name(name) { }

        template<typename AssetT, typename... AssetCtorArgsT>
        requires(std::is_base_of_v<IAsset, AssetT>)
        AssetRef<AssetT> createAssetWithPath(const InterpolatedPath& withPath, AssetCtorArgsT&&... assetCtorArgs) noexcept
        {
            Ref<AssetT> asset = createAssetInstance<AssetT>(std::forward<AssetCtorArgsT>(assetCtorArgs)...);
            asset->m_path = withPath;
            asset->m_storedBy = AssetStorageType::BY_PATH;
            m_assets[hashString(Utils::toUTF8(asset->getPath().resolved().u16string()))][AssetT::type_id] = asset;

            return AssetRef<AssetT>(asset);
        }

        template<typename AssetT, typename... AssetCtorArgsT>
        requires(std::is_base_of_v<IAsset, AssetT>)
        AssetRef<AssetT> createAssetWithAlias(const std::string& withAlias, AssetCtorArgsT&&... assetCtorArgs) noexcept
        {
            Ref<AssetT> asset = createAssetInstance<AssetT>(std::forward<AssetCtorArgsT>(assetCtorArgs)...);
            asset->m_alias = withAlias;
            asset->m_storedBy = AssetStorageType::BY_ALIAS;
            m_assets[hashString(asset->getAlias())][AssetT::type_id] = asset;

            return AssetRef<AssetT>(asset);
        }

        template<typename AssetT, typename... AssetCtorArgsT>
        requires(std::is_base_of_v<IAsset, AssetT>)
        AssetRef<AssetT> createAsset(const InterpolatedPath& withPath, const std::string& withAlias, AssetStorageType storedBy, AssetCtorArgsT&&... assetCtorArgs) noexcept
        {
            Ref<AssetT> asset = createAssetInstance<AssetT>(std::forward<AssetCtorArgsT>(assetCtorArgs)...);
            asset->m_path = withPath;
            asset->m_alias = withAlias;
            asset->m_storedBy = storedBy;

            switch(storedBy)
            {
                case AssetStorageType::BY_PATH:
                    m_assets[hashString(Utils::toUTF8(asset->getPath().resolved().u16string()))][AssetT::type_id] = asset;
                    break;
                case AssetStorageType::BY_ALIAS:
                    m_assets[hashString(asset->getAlias())][AssetT::type_id] = asset;
                    break;
            }

            return AssetRef<AssetT>(asset);
        }

        template<typename AssetT, typename... AssetCtorArgsT>
        Ref<AssetT> createAssetInstance(AssetCtorArgsT&&... assetCtorArgs) noexcept
        {
            Ref<AssetT> asset;
            if constexpr(requires { AssetT::template createRefInstance<AssetCtorArgsT...>; })
            {
                asset = AssetT::createRefInstance(std::forward<AssetCtorArgsT>(assetCtorArgs)...);
                asset->m_parentAssetManager = shared_from_this();

                return asset;
            }
            else if constexpr(!std::is_abstract_v<AssetT>)
            {
                asset = MakeRef<AssetT>();
                asset->m_parentAssetManager = shared_from_this();

                return asset;
            }
            else
            {
                static_assert(always_false<AssetT>::value, "Can not create asset of abstract type! (You can implement createRefInstance function in your asset class to create custom assets).");
            }
        }

        void distributeAsset(const Ref<IAsset>& asset,
                             const InterpolatedPath& path,
                             AssetsLoadPolicy loadPolicy,
                             const Ref<Threading::Thread>& lazyLoadInThread) noexcept
        {
            switch(loadPolicy)
            {
                case SINGLE_THREADED:
                {
                    if(!asset->m_useDataSerde)
                    {
                        asset->load(path.raw());
                    }
                    else
                    {
                        asset->loadFromBinaryFile(this);
                    }

                    asset->lazyLoad();
                    break;
                }
                case PARALLEL_THEN_LAZYLOAD:
                {
                    auto loadInThread = m_threadsPool.getThread();
                    // loadInThread->m_autoJoinIfNotBusy = true;
                    auto loadAssetTask = MakeRef<Threading::Task>();
        
                    loadAssetTask->setOnExecuteCallback([this, asset, path]() {
                        if(!asset->m_useDataSerde)
                        {
                            asset->load(path.raw());
                        }
                        else
                        {
                            asset->loadFromBinaryFile(this);
                        }
                    });
                    
                    if(lazyLoadInThread)
                    {
                        loadAssetTask->setOnExecutedCallback([asset]() {
                            asset->lazyLoad();
                        }, lazyLoadInThread);
                    }
                    
                    loadInThread->addTask(loadAssetTask);
                    loadInThread->start();
                    
                    break;
                }
                case PARALLEL_NO_LAZYLOAD:
                {
                    auto loadInThread = m_threadsPool.getThread();
                    // loadInThread->m_autoJoinIfNotBusy = true;
                    auto loadAssetTask = MakeRef<Threading::Task>();

                    loadAssetTask->setOnExecuteCallback([this, asset, path]() {
                        if(!asset->m_useDataSerde)
                        {
                            asset->load(path.raw());
                        }
                        else
                        {
                            asset->loadFromBinaryFile(this);
                        }
                    });
                    
                    loadInThread->addTask(loadAssetTask);
                    loadInThread->start();
                    
                    break;
                }
            }
            
            if(lazyLoadInThread)
            {
                // lazyLoadInThread->processFinishedTasks();
            }
        }
        
        std::mutex m_mutex;

        std::string m_name;

        AssetsPackage m_package;

        // first - hash of path or alias
        // second - asset by path but loaded in different formats
        //      first - type name id
        //      second - asset
        std::unordered_map<size_t, std::unordered_map<size_t, Ref<IAsset>>> m_assets;
        
        Threading::BaseThreadsPool<Threading::LeastTasksCount> m_threadsPool { 2, false };

        /// This event is using for resolve references of member assets after package deserialization.
        /// This event is called after package deserialization.
        /// You can subscribe to this event to resolve member assets references.
        /// @param assetManager AssetManager for which new assets were deserialized.
        static inline Event<void(AssetManager* assetManager)> onMemberAssetsReferencesResolve;

        static inline Ref<AssetManager> m_instance;

        static inline std::unordered_map<std::string, Ref<AssetManager>> s_allAssetsManagers;
    };
}

#endif // SUNGEARENGINE_ASSETMANAGER_H
