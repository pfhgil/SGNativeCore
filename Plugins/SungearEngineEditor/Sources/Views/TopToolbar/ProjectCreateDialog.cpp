//
// Created by ilya on 30.04.24.
//

#include <imgui.h>
#include <imgui_stdlib.h>
#include <SGCore/Graphics/API/ITexture2D.h>
#include <nfd.h>
#include <SGCore/Input/InputManager.h>
#include <SGCore/PluginsSystem/PluginsManager.h>
#include <SGCore/Memory/Assets/SVGImage.h>
#include <SGCore/Annotations/AnnotationsProcessor.h>
#include <SGCore/Annotations/StandardCodeGeneration/SerializersGeneration/SerdeSpecsGenerator.h>

#include "ProjectCreateDialog.h"
#include "SungearEngineEditor.h"
#include "ImGuiUtils.h"

#include "Views/Explorer/DirectoryExplorer.h"
#include "Views/Explorer/DirectoriesTreeExplorer.h"
#include "Project/CodeGen/CodeGeneration.h"
#include "Toolchains//VisualStudioToolchain.h"
#include "Views/DialogWindowsManager.h"

SGE::ProjectCreateDialog::ProjectCreateDialog() noexcept
{
    m_isPopupWindow = true;
    m_enableEscapeToCloseWindow = false;

    const auto buttonsSize = ImVec2(75, 0);

    addButton({
                      .m_text = "OK",
                      .m_name = "OKButton",
                      .isFastClicked = [](auto& self) -> bool {
                          return ImGui::IsKeyPressed(ImGuiKey_Enter);
                      },
                      .onClicked = [this](auto& self, SGCore::ImGuiWrap::IView* parentView) {
                          submit();
                      },
                      .m_color = ImVec4(10 / 255.0f, 80 / 255.0f, 120 / 255.0f, 1),
                      .m_hoveredColor = ImVec4(0 / 255.0f, 70 / 255.0f, 110 / 255.0f, 1),
                      .m_borderColor = { 0, 0, 0, 0 },
                      .m_borderShadowColor = { 0, 0, 0, 0 },
                      .m_size = buttonsSize
              });

    addButton({
                      .m_text = "Cancel",
                      .m_name = "CancelButton",
                      .isFastClicked = [](auto& self) -> bool {
                          return ImGui::IsKeyPressed(ImGuiKey_Escape);
                      },
                      .onClicked = [this](auto& self, SGCore::ImGuiWrap::IView* parentView) {
                          cancel();
                      },
                      .m_size = buttonsSize
              });
}

void SGE::ProjectCreateDialog::renderBody()
{
    switch(m_mode)
    {
        case OPEN:
        {
            m_bodyMinSize = { 450, 40 };
            m_name = "Open Project";
            break;
        }
        case CREATE:
        {
            m_bodyMinSize = { 450, 90 };
            m_name = "Create Project";
            break;
        }
    }

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(1, 3));

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if(ImGui::BeginTable((m_name.getName() + "_Table").c_str(), 3, ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableNextRow();
        {
            ImGui::TableNextColumn();
            ImGui::Text("Location");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 7);
            ImGui::InputText("##DirectoryPath", &m_dirPath);

            if (ImGui::IsItemEdited())
            {
                m_error = "";
            }

            ImGui::TableNextColumn();

            auto folderTexture = StylesManager::getCurrentStyle()->m_folderIcon
                    ->getSpecialization(20, 20)
                    ->getTexture();

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 7);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3);
            if (ImGuiUtils::ImageButton(folderTexture->getTextureNativeHandler(),
                                        ImVec2(folderTexture->getWidth() + 6, folderTexture->getHeight() + 6),
                                        ImVec2(folderTexture->getWidth(), folderTexture->getHeight())).m_isLMBClicked)
            {
                char* dat = m_dirPath.data();
                nfdresult_t result = NFD_PickFolder("", &dat);
                if (result == NFD_OKAY)
                {
                    m_dirPath = dat;
                }
            }
        }

        if (m_mode == FileOpenMode::CREATE)
        {
            ImGui::TableNextRow();

            {
                ImGui::TableNextColumn();
                ImGui::Text("Project Name");

                ImGui::TableNextColumn();
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 7);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::InputText("##FileName", &m_projectName);

                if (ImGui::IsItemEdited())
                {
                    m_error = "";
                }

                ImGui::TableNextColumn();
            }
        }

        if (m_mode == FileOpenMode::CREATE)
        {
            ImGui::TableNextRow();

            {
                ImGui::TableNextColumn();
                ImGui::Text("C++ Standard");

                ImGui::TableNextColumn();
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 7);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::Combo("##CreateProject_CPPStandard", &m_currentSelectedCPPStandard, m_cppStandards, 7);

                ImGui::TableNextColumn();
            }
        }

        ImGui::EndTable();
    }

    ImGui::PopStyleVar();
}

void SGE::ProjectCreateDialog::footerRender()
{
    if(!m_error.empty())
    {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), m_error.c_str());
        ImGui::SameLine();
    }
}

void SGE::ProjectCreateDialog::submit()
{
    if(!std::filesystem::exists(m_dirPath))
    {
        m_error = "This directory does not exist!";
    }
    else if(!std::filesystem::exists(m_dirPath + "/" + m_projectName) && m_mode == FileOpenMode::CREATE)
    {
        std::filesystem::create_directory(m_dirPath + "/" + m_projectName);
        std::string projectSourcesCreateError;
        SGCore::PluginProject pluginProject = SGCore::PluginsManager::createPluginProject(
                m_dirPath, m_projectName,
                m_cppStandards[m_currentSelectedCPPStandard], projectSourcesCreateError);

        if(!projectSourcesCreateError.empty())
        {
            m_error = projectSourcesCreateError;

            std::filesystem::remove(m_dirPath + "/" + m_projectName);

            return;
        }

        SungearEngineEditor::getInstance()->m_currentProject = SGCore::MakeRef<Project>();
        auto currentEditorProject = SungearEngineEditor::getInstance()->m_currentProject;

        // PROJECT SUCCESSFULLY CREATED ==============

        SungearEngineEditor::getInstance()->getMainView()->getDirectoriesTreeExplorer()->m_rootPath = m_dirPath + "/" + m_projectName;
        currentEditorProject->m_pluginProject = pluginProject;

        // PARSING SUNGEAR ENGINE ANNOTATIONS AND GENERATING CODE ==========================

        SGCore::AnnotationsProcessor annotationsProcessor;

        const char* sungearRoot = std::getenv("SUNGEAR_SOURCES_ROOT");
        if(!sungearRoot)
        {
            LOG_E(SGEDITOR_TAG, "Can not build Sungear Engine: missing environment variable 'SUNGEAR_SOURCES_ROOT'.");
            return;
        }
        const std::string sungearRootStr = sungearRoot;
        const std::string sungearPluginsPathStr = sungearRootStr + "/Plugins";

        annotationsProcessor.processAnnotations(sungearRootStr + "/Sources",
                                                {sungearRootStr + "/Sources/SGCore/Annotations/Annotations.h",
                                                 sungearRootStr +
                                                 "/Sources/SGCore/Annotations/AnnotationsProcessor.cpp",
                                                 sungearRootStr +
                                                 "/Sources/SGCore/Annotations/StandardCodeGeneration/SerializersGeneration/SerdeSpecsGenerator.cpp"});

        SGCore::CodeGen::SerdeSpecsGenerator serializersGenerator;
        std::string serializersGenerationError = serializersGenerator.generateSerializers(annotationsProcessor,
                                                                                          sungearRootStr + "/.SG_GENERATED/Serializers.h");

        annotationsProcessor.saveToFile(sungearRootStr + "/.SG_GENERATED/AnnotationsProcessor.json");

        // TODO: MAKE ERROR DIALOG SHOW IF ERROR
        if(!serializersGenerationError.empty())
        {
            spdlog::error("Error while generating serializers for Sungear Engine: {0}", serializersGenerationError);
            std::printf("Error while generating serializers for Sungear Engine: %s\n", serializersGenerationError.c_str());
        }

        // CREATING GENERATED CODE ENTRY POINT

        CodeGeneration::generateCode({ annotationsProcessor }, pluginProject.m_pluginPath);

        // =====================================================================================
        // BUILDING CREATED PROJECT

        if(!EngineSettings::getInstance()->getToolchains().empty())
        {
            Toolchain::ProjectSpecific::buildProject(EngineSettings::getInstance()->getToolchains()[0]);
        }
        else
        {
            // showing warning dialog that no toolchains were added
            auto projectBuiltDialogWindow = DialogWindowsManager::createTwoButtonsWindow("Project Build", "Create Toolchain", "OK");
            projectBuiltDialogWindow.m_level = SGCore::Logger::Level::LVL_WARN;
            projectBuiltDialogWindow.onCustomBodyRenderListener = [currentEditorProject]() {
                ImGui::SameLine();
                ImGui::TextWrapped(fmt::format("The project '{}' cannot be built: no toolchain has been added. "
                                               "Go to the <Engine Settings - Development and Support - Toolchains> and add your first toolchain.", currentEditorProject->m_pluginProject.m_name).c_str());
            };
            DialogWindowsManager::addDialogWindow(projectBuiltDialogWindow);
        }

        // TEST!!!!!

        /*VisualStudioToolchain testToolchain;
        testToolchain.setPath("F:/VisualStudio/IDE");
        testToolchain.m_archType = VCArchType::AMD64;

        testToolchain.buildProject(currentEditorProject.m_pluginProject.m_pluginPath, "release-host");*/

        // currentEditorProject.m_editorHelper.load("")

        // =====================================================================================

        m_projectName.clear();
        m_dirPath.clear();

        m_error = "";

        setActive(false);
    }
    else if(std::filesystem::exists(m_dirPath) && m_mode == FileOpenMode::OPEN)
    {
        SungearEngineEditor::getInstance()->getMainView()->getDirectoriesTreeExplorer()->m_rootPath = m_dirPath;

        m_projectName.clear();
        m_dirPath.clear();

        m_error = "";

        setActive(false);
    }
    else
    {
        m_error = "This project already exists!";
    }
}

void SGE::ProjectCreateDialog::cancel()
{
    m_projectName.clear();
    m_dirPath.clear();

    m_error = "";

    setActive(false);
}
