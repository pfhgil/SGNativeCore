#ifndef SUNGEARENGINE_NODE_H
#define SUNGEARENGINE_NODE_H

#include <entt/entt.hpp>

#include "IMeshData.h"
#include "glm/detail/type_quat.hpp"
#include "SGCore/Scene/Scene.h"
#include "glm/fwd.hpp"

namespace SGCore
{
    class Node
    {
    private:
        using MeshFunc = std::function<void(const entt::entity& parentEntity, const entt::entity& meshEntity)>;
        using EachEntityFunc = std::function<void(const entt::entity& parentEntity)>;

        entt::entity addOnScene(const Ref<Scene>& scene,
                                const std::string& layerName,
                                const EachEntityFunc& eachEntityFunc,
                                const MeshFunc& meshFunc,
                                const bool& rootAdd) noexcept;

    public:
        std::string m_name;

        std::list<Ref<Node>> m_children;
        std::list<Ref<IMeshData>> m_meshesData;

        glm::vec3 m_position { 1.0 };
        glm::quat m_rotationQuaternion = glm::identity<glm::quat>();
        glm::vec3 m_scale { 1.0 };

        void addOnScene(const Ref<Scene>& scene,
                        const std::string& layerName,
                        const EachEntityFunc& eachEntityFunc,
                        const MeshFunc& meshFunc) noexcept;
        void addOnScene(const Ref<Scene>& scene, const std::string& layerName, const EachEntityFunc& eachEntityFunc) noexcept;
        void addOnScene(const Ref<Scene>& scene, const std::string& layerName, const MeshFunc& meshFunc) noexcept;
        void addOnScene(const Ref<Scene>& scene, const std::string& layerName) noexcept;
    };
}

#endif //SUNGEARENGINE_NODE_H
