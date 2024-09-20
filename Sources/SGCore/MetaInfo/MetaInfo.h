//
// Created by ilya on 24.06.24.
//

#ifndef SUNGEARENGINE_METAINFO_H
#define SUNGEARENGINE_METAINFO_H

#include "SGCore/Utils/Utils.h"

namespace SGCore
{
    struct MetaInfo
    {
        struct Meta
        {
            Meta() = default;
            Meta(const Meta&) noexcept = default;
            Meta(Meta&&) noexcept = default;
            /*explicit Annotation(const std::string& value)
            {
                m_value = value;
            }*/

            Meta& operator[](const std::string& childName) noexcept
            {
                auto it = std::find_if(m_children.begin(), m_children.end(), [&childName](const auto& child) {
                    return childName == child.first;
                });

                if(it == m_children.end())
                {
                    m_children.push_back({ childName, { } });
                    return m_children.rbegin()->second;
                }
                else
                {
                    return it->second;
                }
            }

            [[nodiscard]] bool hasChild(const std::string& childName) const noexcept
            {
                auto it = std::find_if(m_children.begin(), m_children.end(), [&childName](const auto& child) {
                    return childName == child.first;
                });

                return it != m_children.end();
            }

            Meta& operator=(const std::string& value) noexcept
            {
                m_value = value;
                return *this;
            }

            Meta& operator=(const Meta&) noexcept = default;
            Meta& operator=(Meta&&) noexcept = default;

            [[nodiscard]] std::string getValue() const noexcept
            {
                return m_value;
            }

            auto& getChildren() noexcept
            {
                return m_children;
            }

        private:
            std::string m_value;
            std::vector<std::pair<std::string, Meta>> m_children;
        };

        SG_NOINLINE static auto& getMeta() noexcept
        {
            return m_meta;
        }

        static void addStandardMetaInfo() noexcept
        {
            // SGCore::AudioSource
            {
                Meta meta;
                meta["fullName"] = "SGCore::AudioSource";
                meta["type"] = "component";

                meta["members"]["m_rolloffFactor"]["setter"] = "setRolloffFactor";
                meta["members"]["m_rolloffFactor"]["getter"] = "getRolloffFactor";

                meta["members"]["m_pitch"]["setter"] = "setPitch";
                meta["members"]["m_pitch"]["getter"] = "getPitch";

                meta["members"]["m_gain"]["setter"] = "setGain";
                meta["members"]["m_gain"]["getter"] = "getGain";

                meta["members"]["m_direction"]["setter"] = "setDirection";
                meta["members"]["m_direction"]["getter"] = "getDirection";

                meta["members"]["m_velocity"]["setter"] = "setVelocity";
                meta["members"]["m_velocity"]["getter"] = "getVelocity";

                meta["members"]["m_position"]["setter"] = "setPosition";
                meta["members"]["m_position"]["getter"] = "getPosition";

                meta["members"]["m_isLooping"]["setter"] = "setIsLooping";
                meta["members"]["m_isLooping"]["getter"] = "isLooping";

                meta["members"]["m_attachedAudioTrack"];

                meta["members"]["m_lastState"]["setter"] = "setState";
                meta["members"]["m_lastState"]["getter"] = "getState";

                meta["members"]["m_type"]["setter"] = "setType";
                meta["members"]["m_type"]["getter"] = "getType";

                m_meta["structs"].push_back(meta);
            }

            // SGCore::UniqueName
            {
                Meta meta;
                meta["fullName"] = "SGCore::UniqueName";

                meta["members"]["m_rawName"];
                meta["members"]["m_uniqueID"];
                meta["members"]["m_name"];

                m_meta["structs"].push_back(meta);
            }

            // SGCore::UniqueNameWrapper
            {
                Meta meta;
                meta["fullName"] = "SGCore::UniqueNameWrapper";

                meta["members"]["m_uniqueName"];

                m_meta["structs"].push_back(meta);
            }

            // SGCore::AABB
            {
                Meta meta;
                meta["fullName"] = "SGCore::AABB";
                meta["type"] = "component";
                meta["templates"]["ScalarT"] = "typename";

                meta["members"]["m_min"];
                meta["members"]["m_max"];

                m_meta["structs"].push_back(meta);
            }

            // SGCore::Layer
            {
                Meta meta;
                meta["fullName"] = "SGCore::Layer";

                meta["members"]["m_name"];
                meta["members"]["m_isOpaque"];
                meta["members"]["m_index"];

                m_meta["structs"].push_back(meta);
            }

            // SGCore::TransformBase
            {
                Meta meta;
                meta["fullName"] = "SGCore::TransformBase";

                meta["members"]["m_blockTranslation"];
                meta["members"]["m_blockRotation"];
                meta["members"]["m_blockScale"];
                meta["members"]["m_aabb"];
                meta["members"]["m_position"];
                meta["members"]["m_rotation"];
                meta["members"]["m_scale"];
                meta["members"]["m_left"];
                meta["members"]["m_forward"];
                meta["members"]["m_up"];

                m_meta["structs"].push_back(meta);
            }

            // SGCore::Transform
            {
                Meta meta;
                meta["fullName"] = "SGCore::Transform";
                meta["type"] = "component";
                meta["getFromRegistryBy"] = "SGCore::Ref<SGCore::Transform>";

                meta["members"]["m_finalTransform"];
                meta["members"]["m_ownTransform"];
                meta["members"]["m_followParentTRS"];

                m_meta["structs"].push_back(meta);
            }

            // SGCore::EntityBaseInfo
            {
                Meta meta;
                meta["fullName"] = "SGCore::EntityBaseInfo";
                meta["type"] = "component";

                meta["extends"]["SGCore::UniqueNameWrapper"];

                meta["members"]["m_parent"];

                m_meta["structs"].push_back(meta);
            }

            // SGCore::MeshDataRenderInfo
            {
                Meta meta;
                meta["fullName"] = "SGCore::MeshDataRenderInfo";

                meta["members"]["m_useIndices"];
                meta["members"]["m_enableFacesCulling"];
                meta["members"]["m_facesCullingFaceType"];
                meta["members"]["m_facesCullingPolygonsOrder"];
                meta["members"]["m_drawMode"];
                meta["members"]["m_linesWidth"];
                meta["members"]["m_pointsSize"];

                m_meta["structs"].push_back(meta);
            }

            // SGCore::RenderingBase
            {
                Meta meta;
                meta["fullName"] = "SGCore::RenderingBase";
                meta["type"] = "component";

                meta["members"]["m_fov"];
                meta["members"]["m_aspect"];
                meta["members"]["m_zNear"];
                meta["members"]["m_zFar"];
                meta["members"]["m_left"];
                meta["members"]["m_right"];
                meta["members"]["m_bottom"];
                meta["members"]["m_top"];
                meta["members"]["m_projectionMatrix"];
                meta["members"]["m_orthographicMatrix"];
                meta["members"]["m_viewMatrix"];
                meta["members"]["m_projectionSpaceMatrix"];
                meta["members"]["m_orthographicSpaceMatrix"];

                m_meta["structs"].push_back(meta);
            }

            // SGCore::Camera3D
            {
                Meta meta;
                meta["fullName"] = "SGCore::Camera3D";
                meta["type"] = "component";

                m_meta["structs"].push_back(meta);
            }

            // SGCore::GizmoBase
            {
                Meta meta;
                meta["fullName"] = "SGCore::GizmoBase";

                meta["members"]["m_color"];
                meta["members"]["m_meshBase"];

                m_meta["structs"].push_back(meta);
            }

            // SGCore::SphereGizmo
            {
                Meta meta;
                meta["fullName"] = "SGCore::SphereGizmo";
                meta["type"] = "component";

                meta["members"]["m_base"];
                meta["members"]["m_radius"];
                meta["members"]["m_angleIncrement"];

                m_meta["structs"].push_back(meta);
            }

            // SGCore::BoxGizmo
            {
                Meta meta;
                meta["fullName"] = "SGCore::BoxGizmo";
                meta["type"] = "component";

                meta["members"]["m_base"];
                meta["members"]["m_size"];
                meta["members"]["m_lastSize"];

                m_meta["structs"].push_back(meta);
            }

            // SGCore::LineGizmo
            {
                Meta meta;
                meta["fullName"] = "SGCore::LineGizmo";
                meta["type"] = "component";

                meta["members"]["m_base"];
                meta["members"]["m_start"];
                meta["members"]["m_end"];

                m_meta["structs"].push_back(meta);
            }

            // SGCore::MeshBase
            {
                Meta meta;
                meta["fullName"] = "SGCore::MeshBase";

                meta["members"]["m_meshDataRenderInfo"];

                meta["members"]["m_meshData"]["setter"] = "setMeshData";
                meta["members"]["m_meshData"]["getter"] = "getMeshData";

                meta["members"]["m_material"]["setter"] = "setMaterial";
                meta["members"]["m_material"]["getter"] = "getMaterial";

                m_meta["structs"].push_back(meta);
            }

            // SGCore::LightBase
            {
                Meta meta;
                meta["fullName"] = "SGCore::LightBase";

                meta["members"]["m_color"];
                meta["members"]["m_intensity"];
                meta["members"]["m_samplesCount"];

                m_meta["structs"].push_back(meta);
            }

            // SGCore::DirectionalLight
            {
                Meta meta;
                meta["fullName"] = "SGCore::DirectionalLight";
                meta["type"] = "component";

                meta["members"]["m_base"];

                m_meta["structs"].push_back(meta);
            }

            // SGCore::DirectionalLight
            {
                Meta meta;
                meta["fullName"] = "SGCore::Atmosphere";
                meta["type"] = "component";

                meta["members"]["m_sunPosition"];

                meta["members"]["m_sunAmbient"];
                meta["members"]["m_lastSunAmbient"];

                meta["members"]["m_sunRotation"];
                meta["members"]["m_lastSunRotation"];

                meta["members"]["m_rayleighScatteringCoeff"];
                meta["members"]["m_lastRayleighScatteringCoeff"];

                meta["members"]["m_mieScatteringCoeff"];
                meta["members"]["m_lastMieScatteringCoeff"];

                meta["members"]["m_rayleighScaleHeight"];
                meta["members"]["m_lastRayleighScaleHeight"];

                meta["members"]["m_mieScaleHeight"];
                meta["members"]["m_lastMieScaleHeight"];

                meta["members"]["m_sunIntensity"];
                meta["members"]["m_lastSunIntensity"];

                meta["members"]["m_planetRadius"];
                meta["members"]["m_lastPlanetRadius"];

                meta["members"]["m_atmosphereRadius"];
                meta["members"]["m_lastAtmosphereRadius"];

                meta["members"]["m_miePreferredScatteringDirection"];
                meta["members"]["m_lastMiePreferredScatteringDirection"];

                meta["members"]["m_rayOrigin"];
                meta["members"]["m_lastRayOrigin"];

                m_meta["structs"].push_back(meta);
            }

            // SGCore::Controllable3D
            {
                Meta meta;
                meta["fullName"] = "SGCore::Controllable3D";
                meta["type"] = "component";

                meta["members"]["m_movementSpeed"];
                meta["members"]["m_rotationSensitive"];

                m_meta["structs"].push_back(meta);
            }

            // SGCore::UICamera
            {
                Meta meta;
                meta["fullName"] = "SGCore::UICamera";
                meta["type"] = "component";

                m_meta["structs"].push_back(meta);
            }
        }

        static void clearAllMeta() noexcept
        {
            m_meta.clear();
        }

    private:
        static inline std::unordered_map<std::string, std::vector<Meta>> m_meta;
    };
}

#endif //SUNGEARENGINE_METAINFO_H
