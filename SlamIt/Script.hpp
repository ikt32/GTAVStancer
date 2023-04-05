#pragma once
#include "StanceScript.hpp"
#include "ScriptMenu.hpp"
#include "ScriptSettings.hpp"

namespace VStancer {
    void ScriptMain();

    std::vector<CScriptMenu<CStanceScript>::CSubmenu> BuildMenu();

    CScriptSettings& GetSettings();
    const std::vector<std::shared_ptr<CStanceScript>>& GetScripts();

    const std::vector<CConfig>& GetConfigs();

    uint32_t LoadConfigs();
    void SaveConfigs();

    void SwitchMode(bool playerOnly);
}
