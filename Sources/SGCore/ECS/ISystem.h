//
// Created by stuka on 02.05.2023.
//

#ifndef NATIVECORE_ISYSTEM_H
#define NATIVECORE_ISYSTEM_H

#include <functional>
#include <map>
#include <source_location>
#include <GLFW/glfw3.h>

#include "Scene.h"
#include "SGCore/Patterns/Marker.h"
#include "Transformations/Transform.h"
#include "SGCore/Patterns/Singleton.h"

#include "ComponentsCollection.h"

#include "SGCore/Main/CoreGlobals.h"

namespace SGCore
{
    struct LayerCachedEntities
    {
        std::unordered_map<Ref<Entity>, ComponentsCollection> m_cachedEntities;
    };

    class ISystem
    {
        friend class ECSWorld;
    public:
        bool m_active = true;

        virtual void fixedUpdate(const Ref<Scene>& scene) { }
        virtual void update(const Ref<Scene>& scene) { }

        virtual void fixedUpdate(const Ref<Scene>& scene, const Ref<Entity>& entity) { }
        virtual void update(const Ref<Scene>& scene, const Ref<Entity>& entity) { }

        virtual void cacheEntity(const Ref<Entity>& entity) { }

        template<typename... ComponentsT>
        requires(std::is_base_of_v<IComponent, ComponentsT> && ...)
        void cacheEntityComponents(const Ref<Entity>& entity,
                                   const std::function<bool()>& willCachePredicate,
                                   const std::function<bool(const Ref<IComponent>& component)>& willCacheComponentPredicate)
        {
            double t0 = glfwGetTime();

            if(!willCachePredicate()) return;

            auto entityLayer = entity->getLayer();
            if(!entityLayer) return;

            // is components of all ComponentsT... exists
            bool componentsSetExistsInEntity = true;
            Utils::Utils::forTypes<ComponentsT...>([&entity, &componentsSetExistsInEntity](auto t)
                                                   {
                                                       using type = typename decltype(t)::type;

                                                       if(!entity->getComponent<type>())
                                                       {
                                                           componentsSetExistsInEntity = false;
                                                           return;
                                                       }
                                                   });

            // if not exists that we wil not cache components of this entity
            if(!componentsSetExistsInEntity) return;

            auto& foundComponentsCollection = m_cachedEntities[entityLayer].m_cachedEntities[entity];

            Utils::Utils::forTypes<ComponentsT...>([&entity, &foundComponentsCollection, &willCacheComponentPredicate](auto t)
                                                   {
                                                       using type = typename decltype(t)::type;

                                                       // if component already exists in components collection then we wont cache
                                                       if (!foundComponentsCollection.getComponent<type>())
                                                       {
                                                           auto entityComponentsList = entity->getComponents<type>();
                                                           for (const auto& component : entityComponentsList)
                                                           {
                                                               if(willCacheComponentPredicate(component))
                                                               {
                                                                   foundComponentsCollection.addComponent(component);
                                                               }
                                                           }
                                                       }
                                                   });

            double t1 = glfwGetTime();

            std::cout << "ms: " << std::to_string((t1 - t0) * 1000.0) << std::endl;
        }

        template<typename... ComponentsT>
        requires(std::is_base_of_v<IComponent, ComponentsT> && ...)
        void cacheEntityComponents(const Ref<Entity>& entity,
                                   const std::function<bool()>& willCachePredicate)
        {
            cacheEntityComponents<ComponentsT...>(entity, willCachePredicate, [](const Ref<IComponent>& component) { return true; });
        }

        template<typename... ComponentsT>
        requires(std::is_base_of_v<IComponent, ComponentsT> && ...)
        void cacheEntityComponents(const Ref<Entity>& entity,
                                   const std::function<bool(const Ref<IComponent>& component)>& willCacheComponentPredicate)
        {
            cacheEntityComponents<ComponentsT...>(entity, []() { return true; }, willCacheComponentPredicate);
        }

        template<typename... ComponentsT>
        requires(std::is_base_of_v<IComponent, ComponentsT> && ...)
        void cacheEntityComponents(const Ref<Entity>& entity)
        {
            cacheEntityComponents<ComponentsT...>(entity, []() { return true; }, [](const Ref<IComponent>& component) { return true; });
        }

        auto& getCachedEntities() noexcept
        {
            return m_cachedEntities;
        }

        template<typename Func, typename... Args>
        void addFunctionToFixedUpdateQuery(const std::string& funcUUID, const Func& f, const Args&... args)
        {
            std::function<bool()> bindFunc = [f, args...]() { return f(args...); };
            m_fixedUpdateFunctionsQuery[funcUUID] = bindFunc;
        }

        template<typename Func, typename... Args>
        void addFunctionToUpdateQuery(const std::string& funcUUID, const Func& f, const Args&... args)
        {
            std::function<bool()> bindFunc = [f, args...]() { return f(args...); };
            m_updateFunctionsQuery[funcUUID] = (bindFunc);
        }

        double getUpdateFunctionExecutionTime() const noexcept;
        double getFixedUpdateFunctionExecutionTime() const noexcept;

    protected:
        double m_update_executionTime = 0.0;
        double m_fixedUpdate_executionTime = 0.0;

        std::unordered_map<std::string, std::function<bool()>> m_fixedUpdateFunctionsQuery;
        std::unordered_map<std::string, std::function<bool()>> m_updateFunctionsQuery;

        std::map<Ref<Layer>, LayerCachedEntities, LayersComparator> m_cachedEntities;
    };
}

#endif //NATIVECORE_ISYSTEM_H
