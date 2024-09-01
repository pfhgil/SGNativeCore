#include "SungearEngineEditor.h"
#include "PluginMain.h"
#include "Styles/StylesManager.h"

#include "SGCore/Utils/Singleton.h"
#include "Views/DialogWindowsManager.h"
#include <SGCore/Main/CoreMain.h>
#include <SGCore/Input/InputManager.h>
#include <SGCore/ImGuiWrap/ImGuiLayer.h>
#include <SGCore/Logger/Logger.h>

SGE::SungearEngineEditor::~SungearEngineEditor()
{
    SGCore::ImGuiWrap::IView::getRoot()->removeChild(m_mainView);
}

std::string SGE::SungearEngineEditor::onConstruct(const std::vector<std::string>& args)
{
    ImGui::SetCurrentContext(SGCore::ImGuiWrap::ImGuiLayer::getCurrentContext());

	m_name = "SungearEngineEditor";
	m_version = "1.0.0";

    EngineSettings::getInstance()->load("configs/engine_settings.json");

    StylesManager::init();
    DialogWindowsManager::init();
    
    m_mainView = SGCore::MakeRef<MainView>();
    SGCore::ImGuiWrap::IView::getRoot()->addChild(m_mainView);

    LOG_I(SGEDITOR_TAG, "Info msg")
    LOG_D(SGEDITOR_TAG, "Debug msg")
    LOG_W(SGEDITOR_TAG, "Warning msg")
    LOG_E(SGEDITOR_TAG, "Error msg")
    LOG_C(SGEDITOR_TAG, "Critical msg")

	// No error.
	return "";
}

void SGE::SungearEngineEditor::update(const double& dt, const double& fixedDt)
{
    if(SGCore::InputManager::getMainInputListener()->keyboardKeyReleased(SGCore::KeyboardKey::KEY_R))
    {
        /*try
        {
            // CRASH HERE ======================
            void* ptr = nullptr;
            *((int*) ptr) = 5;
            // ==================================
        }
        catch(const std::exception& e)
        {
            std::printf("Exception: %s\n", e.what());
        }*/
        /*std::ofstream ofs(std::filesystem::current_path().string() + "/imgui.ini", std::ios::trunc | std::ios::out);

        SGCore::ImGuiWrap::ImGuiLayer::destroy();
        SGCore::ImGuiWrap::ImGuiLayer::init();
        ImGui::LoadIniSettingsFromDisk((std::filesystem::current_path().string() + "/imgui.ini").c_str());
        StylesManager::init();*/
    }
}

void SGE::SungearEngineEditor::fixedUpdate(const double& dt, const double& fixedDt)
{

}

SGCore::Ref<SGE::MainView> SGE::SungearEngineEditor::getMainView() const noexcept
{
    return m_mainView;
}

SGCore::Ref<SGE::SungearEngineEditor> SGE::SungearEngineEditor::getInstance() noexcept
{
    return s_SungearEngineEditorInstance;
}


