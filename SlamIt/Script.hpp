#pragma once
#include "StanceScript.hpp"
#include "ScriptMenu.hpp"
#include "ScriptSettings.hpp"

namespace VStancer {
    void ScriptMain();

    std::vector<CScriptMenu<CStanceScript>::CSubmenu> BuildMenu();

    CScriptSettings* GetSettings();

    std::vector<CConfig>& GetConfigs();

    uint32_t LoadConfigs();
    void SaveConfigs();
}
