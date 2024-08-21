//
// Created by Ilya on 21.08.2024.
//

#include "LogsWindow.h"
#include "imgui_internal.h"

#include <SGCore/Logger/Logger.h>

bool SGE::LogsWindow::begin()
{
    return IView::begin();
}

void SGE::LogsWindow::renderBody()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5, 0.5));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 0));

    ImGuiWindowClass windowClass;
    windowClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;
    ImGui::SetNextWindowClass(&windowClass);

    ImGui::Begin("LogsView");

    for(const SGCore::LogMessage& logMessage : SGCore::Logger::getDefaultLogger()->getAllMessages())
    {
        switch (logMessage.m_type)
        {
            case SGCore::LogMessageType::T_INFO:
                ImGui::TextColored({ 1.0, 1.0, 1.0, 1.0 }, logMessage.m_message.c_str());
                break;
            case SGCore::LogMessageType::T_DEBUG:
                ImGui::TextColored({ 0.0, 1.0, 0.0, 1.0 }, logMessage.m_message.c_str());
                break;
            case SGCore::LogMessageType::T_WARN:
                ImGui::TextColored({ 233.0f / 255.0f, 213.0f / 255.0f, 2.0f / 255.0f, 1.0 }, logMessage.m_message.c_str());
                break;
            case SGCore::LogMessageType::T_ERROR:
                ImGui::TextColored({ 1.0, 0.0, 0.0, 1.0 }, logMessage.m_message.c_str());
                break;
            case SGCore::LogMessageType::T_CRITICAL:
                ImGui::TextColored({ 1.0, 0.0, 0.0, 1.0 }, logMessage.m_message.c_str());
                break;
        }
    }

    ImGui::End();

    ImGui::PopStyleVar(2);
}

void SGE::LogsWindow::end()
{
    IView::end();
}
