#include "PluginMain.h"

SG_NOMANGLING SG_DLLEXPORT SGCore::Ref<SGCore::IPlugin> SGPluginMain()
{
	return SGE::SungearEngineEditor::getInstance();
}

int main()
{
    return 0;
}
