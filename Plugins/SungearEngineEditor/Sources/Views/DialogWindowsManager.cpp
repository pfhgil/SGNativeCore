//
// Created by stuka on 01.09.2024.
//

#include "DialogWindowsManager.h"
#include "Styles/StylesManager.h"

void SGE::DialogWindowsManager::init() noexcept
{
    m_infoIcon = StylesManager::getCurrentStyle()->m_infoIcon
            ->getSpecialization(24, 24)
            ->getTexture();

    m_debugIcon = StylesManager::getCurrentStyle()->m_greenBugIcon
            ->getSpecialization(24, 24)
            ->getTexture();

    m_warnIcon = StylesManager::getCurrentStyle()->m_warningIcon
            ->getSpecialization(24, 24)
            ->getTexture();

    m_errorIcon = StylesManager::getCurrentStyle()->m_errorIcon
            ->getSpecialization(24, 24)
            ->getTexture();

    m_criticalIcon = StylesManager::getCurrentStyle()->m_criticalIcon
            ->getSpecialization(24, 24)
            ->getTexture();

    m_windowsNamesManager = SGCore::MakeRef<SGCore::UniqueNamesManager>();
}

void SGE::DialogWindowsManager::renderWindows() noexcept
{
    auto it = m_dialogWindows.begin();
    while(it != m_dialogWindows.end())
    {
        if(!it->isActive())
        {
            it = m_dialogWindows.erase(it);
            continue;
        }

        it->render();

        ++it;
    }
}

void SGE::DialogWindowsManager::addDialogWindow(SGE::DialogWindow& dialogWindow) noexcept
{
    m_dialogWindows.push_back(dialogWindow);
    auto& lastDialogWnd = *m_dialogWindows.rbegin();
    // adding listener to onRenderBody for rendering image
    SGCore::EventListener<void()> onRenderListener = [&lastDialogWnd]() {
        switch(lastDialogWnd.m_level)
        {
            case SGCore::Logger::Level::LVL_INFO:
            {
                LOG_I("Hui", "DIALOG WINDOW WITH LVL INFO")
                ImGui::Image(m_infoIcon->getTextureNativeHandler(),
                             ImVec2(m_infoIcon->getWidth(), m_infoIcon->getHeight()));

                break;
            }
            case SGCore::Logger::Level::LVL_DEBUG:break;
            case SGCore::Logger::Level::LVL_WARN:break;
            case SGCore::Logger::Level::LVL_ERROR:break;
            case SGCore::Logger::Level::LVL_CRITICAL:break;
        }
    };
    onRenderListener.m_priority = 0;

    lastDialogWnd.onRenderBody += onRenderListener;
}

const std::vector<SGE::DialogWindow>& SGE::DialogWindowsManager::getDialogWindows() noexcept
{
    return m_dialogWindows;
}

SGE::DialogWindow
SGE::DialogWindowsManager::createThreeButtonsWindow(const std::string& windowName,
                                                    const std::string& firstButtonText,
                                                    const std::string& secondButtonText,
                                                    const std::string& thirdButtonText) noexcept
{
    DialogWindow window;

    window.m_name.attachToManager(m_windowsNamesManager);
    window.m_name = windowName;

    window.addButton({
                             .m_text = firstButtonText,
                             .m_name = firstButtonText,
                             .isFastClicked = [](auto& self) -> bool
                             {
                                 return ImGui::IsKeyPressed(ImGuiKey_Enter);
                             },
                             .onClicked = [](auto& self, SGCore::ImGuiWrap::IView* parentView)
                             {
                                 parentView->setActive(false);
                             },
                             .m_color = ImVec4(10 / 255.0f, 80 / 255.0f, 120 / 255.0f, 1),
                             .m_hoveredColor = ImVec4(0 / 255.0f, 70 / 255.0f, 110 / 255.0f, 1),
                             .m_borderColor = {0, 0, 0, 0},
                             .m_borderShadowColor = {0, 0, 0, 0},
                             .m_size = {75, 0}
                     }
    );

    window.addButton({
                             .m_text = secondButtonText,
                             .m_name = secondButtonText,
                             .onClicked = [](auto& self, SGCore::ImGuiWrap::IView* parentView)
                             {
                                 parentView->setActive(false);
                             },
                             .m_size = {75, 0}
                     }
    );

    window.addButton({
                             .m_text = thirdButtonText,
                             .m_name = thirdButtonText,
                             .isFastClicked = [](auto& self) -> bool
                             {
                                 return ImGui::IsKeyPressed(ImGuiKey_Escape);
                             },
                             .onClicked = [](auto& self, SGCore::ImGuiWrap::IView* parentView)
                             {
                                 parentView->setActive(false);
                             },
                             .m_size = {75, 0}
                     }
    );

    /*Button firstButton;
    firstButton.m_text = firstButtonText;
    firstButton.onClicked = [](Button& self, SGCore::ImGuiWrap::IView* parentView) {
        // closing parent window
        parentView->setActive(false);
    };

    Button secondButton;
    secondButton.m_text = secondButtonText;
    secondButton.onClicked = [](Button& self, SGCore::ImGuiWrap::IView* parentView) {
        // closing parent window
        parentView->setActive(false);
    };

    Button thirdButton;
    thirdButton.m_text = thirdButtonText;
    thirdButton.onClicked = [](Button& self, SGCore::ImGuiWrap::IView* parentView) {
        // closing parent window
        parentView->setActive(false);
    };*/

    /*window.addButton(firstButton);
    window.addButton(secondButton);
    window.addButton(thirdButton);*/

    return window;
}

SGE::DialogWindow
SGE::DialogWindowsManager::createTwoButtonsWindow(const std::string& windowName,
                                                  const std::string& firstButtonText,
                                                  const std::string& secondButtonText) noexcept
{
    DialogWindow window;

    window.m_name.attachToManager(m_windowsNamesManager);
    window.m_name = windowName;

    window.addButton({
                             .m_text = firstButtonText,
                             .m_name = firstButtonText,
                             .isFastClicked = [](auto& self) -> bool
                             {
                                 return ImGui::IsKeyPressed(ImGuiKey_Enter);
                             },
                             .onClicked = [](auto& self, SGCore::ImGuiWrap::IView* parentView)
                             {
                                 parentView->setActive(false);
                             },
                             .m_color = ImVec4(10 / 255.0f, 80 / 255.0f, 120 / 255.0f, 1),
                             .m_hoveredColor = ImVec4(0 / 255.0f, 70 / 255.0f, 110 / 255.0f, 1),
                             .m_borderColor = {0, 0, 0, 0},
                             .m_borderShadowColor = {0, 0, 0, 0},
                             .m_size = {75, 0}
                     }
    );

    window.addButton({
                             .m_text = secondButtonText,
                             .m_name = secondButtonText,
                             .isFastClicked = [](auto& self) -> bool
                             {
                                 return ImGui::IsKeyPressed(ImGuiKey_Escape);
                             },
                             .onClicked = [](auto& self, SGCore::ImGuiWrap::IView* parentView)
                             {
                                 parentView->setActive(false);
                             },
                             .m_size = {75, 0}
                     }
    );

    /*Button firstButton;
    firstButton.m_text = firstButtonText;
    firstButton.onClicked = [](Button& self, SGCore::ImGuiWrap::IView* parentView) {
        // closing parent window
        parentView->setActive(false);
    };

    Button secondButton;
    secondButton.m_text = secondButtonText;
    secondButton.onClicked = [](Button& self, SGCore::ImGuiWrap::IView* parentView) {
        // closing parent window
        parentView->setActive(false);
    };*/

    return window;
}

SGE::DialogWindow SGE::DialogWindowsManager::createOneButtonWindow(const std::string& windowName,
                                                                   const std::string& firstButtonText) noexcept
{
    DialogWindow window;

    window.m_name.attachToManager(m_windowsNamesManager);
    window.m_name = windowName;

    window.addButton({
                             .m_text = firstButtonText,
                             .m_name = firstButtonText,
                             .isFastClicked = [](auto& self) -> bool
                             {
                                 return ImGui::IsKeyPressed(ImGuiKey_Enter);
                             },
                             .onClicked = [](auto& self, SGCore::ImGuiWrap::IView* parentView)
                             {
                                 parentView->setActive(false);
                             },
                             .m_color = ImVec4(10 / 255.0f, 80 / 255.0f, 120 / 255.0f, 1),
                             .m_hoveredColor = ImVec4(0 / 255.0f, 70 / 255.0f, 110 / 255.0f, 1),
                             .m_borderColor = {0, 0, 0, 0},
                             .m_borderShadowColor = {0, 0, 0, 0},
                             .m_size = {75, 0}
                     }
    );

    /*Button firstButton;
    firstButton.m_text = firstButtonText;
    firstButton.onClicked = [](Button& self, SGCore::ImGuiWrap::IView* parentView) {
        // closing parent window
        parentView->setActive(false);
    };*/

    return window;
}
