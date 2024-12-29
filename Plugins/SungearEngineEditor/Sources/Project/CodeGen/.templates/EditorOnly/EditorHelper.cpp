// GENERATED BY Sungear Engine Code Generator. DO NOT EDIT!
// ===============================================================

#include <set>

#include "EditorHelper.h"
#include "../Serde.h"

#include <SGCore/Main/CoreMain.h>
#include <SGCore/Scene/Scene.h>
#include <SGCore/Serde/Components/NonSavable.h>
#include <SGCore/Logger/Logger.h>
#include <SGCore/Scene/EntityBaseInfo.h>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <entt/entt.hpp>

#include <Plugins/SungearEngineEditor/Sources/Views/InspectorView.h>
#include <Plugins/SungearEngineEditor/Sources/SungearEngineEditor.h>

// including all structs
## for struct in structs
## if struct.hasMember(name: "filePath")
#include "{{ struct.filePath }}"
## endif
## endfor

// BARRIER FOR STATIC ANALYZER (FOR HIGHLIGHTING)
void DO_NOT_USE()
{

}

template<SGCore::Serde::FormatType TFormatType>
void onEntitySerialize(SGCore::Serde::SerializableValueView<SGCore::SceneEntitySaveInfo, TFormatType>& entityView,
                       const SGCore::Scene& serializableScene,
                       const SGCore::ECS::entity_t& serializableEntity) noexcept
{
    LOG_I("GENERATED", "Saving entity '{}'...", std::to_underlying(serializableEntity));

    // saving components of serializableEntity

    ## for struct in structs
    ## if struct.hasMember(name: "type") && struct.type.equals(value: "component")
    ## if !struct.hasMember(name: "annotations") || !struct.annotations.hasMember(name: "doNotGenerateSerializationLogic")
    {
        ## if struct.hasMember(name: "ecsRegistrySpecialization")
        auto* component = serializableScene.getECSRegistry()->template tryGet<{{ struct.getFromRegistryBy }}>(serializableEntity);
        ## else
        auto* component = serializableScene.getECSRegistry()->template tryGet<{{ struct.fullName }}>(serializableEntity);
        ## endif

        if(component)
        {
            entityView.getValueContainer().pushBack(*component, serializableEntity, *(serializableScene.getECSRegistry()));
        }
    }
    ## endif
    ## endif
    ## endfor
}

template<SGCore::Serde::FormatType TFormatType>
void onEntityDeserialize(SGCore::Serde::DeserializableValueView<SGCore::SceneEntitySaveInfo, TFormatType>& entityView,
                         const typename SGCore::Serde::FormatInfo<TFormatType>::array_iterator_t& componentsIterator,
                         SGCore::ECS::registry_t& toRegistry) noexcept
{
    const auto& entity = entityView.m_data->m_serializableEntity;

    const auto& currentElementTypeName = entityView.getValueContainer().getMemberTypeName(componentsIterator);

    // iterating through all elements of entityView
    ## for struct in structs
    ## if struct.hasMember(name: "type") && struct.type.equals(value: "component")
    ## if !struct.hasMember(name: "annotations") || !struct.annotations.hasMember(name: "doNotGenerateDeserializationLogic")
    if(currentElementTypeName == SGCore::Serde::SerdeSpec<{{ struct.fullName }}, TFormatType>::type_name)
    {
        ## if struct.hasMember(name: "ecsRegistrySpecialization")
        auto component = entityView.getValueContainer().template getMember<{{ struct.ecsRegistrySpecialization }}>(componentsIterator, entityView.m_data->m_serializableEntity, toRegistry);
        ## else
        auto component = entityView.getValueContainer().template getMember<{{ struct.fullName }}>(componentsIterator, entityView.m_data->m_serializableEntity, toRegistry);
        ## endif

        if(component)
        {
            ## if struct.hasMember(name: "ecsRegistrySpecialization")
            toRegistry.emplace<{{ struct.ecsRegistrySpecialization }}>(entity, std::move(*component));
            ## else
            toRegistry.emplace<{{ struct.fullName }}>(entity, std::move(*component));
            ## endif
        }
    }
    ## endif
    ## endif
    ## endfor
}

template<SGCore::Serde::FormatType TFormatType>
void onSystemSerialize(
        SGCore::Serde::SerializableValueView<SGCore::Scene::systems_container_t, TFormatType>& systemsContainerView,
        const SGCore::Scene& savableScene,
        const SGCore::Ref<SGCore::ISystem>& savableSystem) noexcept
{
    ## for struct in structs
    ## if struct.hasMember(name: "type") && struct.type.equals(value: "system") && struct.hasMember(name: "fullName")
    ## if !struct.hasMember(name: "annotations") || !struct.annotations.hasMember(name: "doNotGenerateSerializationLogic")
    {
        // trying to cast to user-provided type
        auto* castedSystem = dynamic_cast<{{ struct.fullName }}*>(savableSystem.get());

        if(castedSystem)
        {
            // serializing castedSystem with only project-scope derived types checking and all base types serializing
            systemsContainerView.getValueContainer().pushBack(castedSystem);

            return;
        }
    }
    ## endif
    ## endif
    ## endfor
}

template<SGCore::Serde::FormatType TFormatType>
void onSystemDeserialize(
        SGCore::Serde::DeserializableValueView<SGCore::Scene::systems_container_t, TFormatType>& systemsContainerView,
        const typename SGCore::Serde::FormatInfo<TFormatType>::array_iterator_t& systemsIterator) noexcept
{
    // trying to deserialize only user-provided types

    ## for struct in structs
    ## if struct.hasMember(name: "type") && struct.type.equals(value: "system") && struct.hasMember(name: "fullName")
    ## if !struct.hasMember(name: "annotations") || !struct.annotations.hasMember(name: "doNotGenerateDeserializationLogic")
    {
        const auto system = systemsContainerView.getValueContainer().template getMember<SGCore::Ref<{{ struct.fullName }}>>(
                systemsIterator
        );
        if(system)
        {
            LOG_D(SGCORE_TAG, "LOADING SYSTEM {}", std::string(typeid(**system).name()));

            systemsContainerView.m_data->emplace_back(std::move(*system));
        }
    }
    ## endif
    ## endif
    ## endfor
}

void onInit()
{

}

void update(const double& dt, const double& fixedDt)
{

}

void fixedUpdate(const double& dt, const double& fixedDt)
{

}

void onInspectorViewRender()
{
    auto inspectorViewInstance = SGE::SungearEngineEditor::getInstance()->getMainView()->getInspectorView();
    auto currentChosenEntity = SGE::SungearEngineEditor::getInstance()->getMainView()->getInspectorView()->m_currentChosenEntity;
    auto currentSceneRegistry = SGCore::Scene::getCurrentScene()->getECSRegistry();

    // TODO: MAKE PREDICATE THAT IT IS NEEDS TO BE RENDERED
    if(currentSceneRegistry->valid(currentChosenEntity))
    {

    }
}

SG_NOMANGLING SG_DLEXPORT void editorGeneratedCodeEntry()
{
    LOG_I("GENERATED", "Calling editorGeneratedCodeEntry()...");

    SGCore::Scene::getOnEntitySerializeEvent<SGCore::Serde::FormatType::JSON>() += onEntitySerialize<SGCore::Serde::FormatType::JSON>;
    SGCore::Scene::getOnEntityDeserializeEvent<SGCore::Serde::FormatType::JSON>() += onEntityDeserialize<SGCore::Serde::FormatType::JSON>;
    SGCore::Scene::getOnSystemSerializeEvent<SGCore::Serde::FormatType::JSON>() += onSystemSerialize<SGCore::Serde::FormatType::JSON>;
    SGCore::Scene::getOnSystemDeserializeEvent<SGCore::Serde::FormatType::JSON>() += onSystemDeserialize<SGCore::Serde::FormatType::JSON>;
    // TODO: supporting BSON and YAML
    /*SGCore::Scene::getOnEntitySave<SGCore::Serde::FormatType::BSON>() += onEntitySave<SGCore::Serde::FormatType::BSON>;
    SGCore::Scene::getOnEntitySave<SGCore::Serde::FormatType::YAML>() += onEntitySave<SGCore::Serde::FormatType::YAML>;*/

    SGE::SungearEngineEditor::getInstance()->getMainView()->getInspectorView()->onRenderBody += onInspectorViewRender;

    SGCore::CoreMain::onInit += onInit;
    SGCore::CoreMain::getFixedTimer().onUpdate += fixedUpdate;
    SGCore::CoreMain::getRenderTimer().onUpdate += update;
}

SG_NOMANGLING SG_DLEXPORT void editorGeneratedCodeExit()
{
    SGCore::Scene::getOnEntitySerializeEvent<SGCore::Serde::FormatType::JSON>() -= onEntitySerialize<SGCore::Serde::FormatType::JSON>;
    SGCore::Scene::getOnEntityDeserializeEvent<SGCore::Serde::FormatType::JSON>() -= onEntityDeserialize<SGCore::Serde::FormatType::JSON>;
    SGCore::Scene::getOnSystemSerializeEvent<SGCore::Serde::FormatType::JSON>() -= onSystemSerialize<SGCore::Serde::FormatType::JSON>;
    SGCore::Scene::getOnSystemDeserializeEvent<SGCore::Serde::FormatType::JSON>() -= onSystemDeserialize<SGCore::Serde::FormatType::JSON>;
    // TODO: supporting BSON and YAML
    /*SGCore::Scene::getOnEntitySave<SGCore::Serde::FormatType::BSON>() -= onEntitySave<SGCore::Serde::FormatType::BSON>;
    SGCore::Scene::getOnEntitySave<SGCore::Serde::FormatType::YAML>() -= onEntitySave<SGCore::Serde::FormatType::YAML>;*/

    SGE::SungearEngineEditor::getInstance()->getMainView()->getInspectorView()->onRenderBody -= onInspectorViewRender;

    SGCore::CoreMain::onInit -= onInit;
    SGCore::CoreMain::getFixedTimer().onUpdate -= fixedUpdate;
    SGCore::CoreMain::getRenderTimer().onUpdate -= update;
}