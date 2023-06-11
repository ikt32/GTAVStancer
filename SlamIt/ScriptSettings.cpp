#include "ScriptSettings.hpp"
#include "SettingsCommon.hpp"

#include "Util/Logger.hpp"

#include <simpleini/SimpleIni.h>

#define CHECK_LOG_SI_ERROR(result, operation) \
    if (result < 0) { \
        LOG(ERROR, "[Settings] {} Failed to {}, SI_Error [{}]", \
        __FUNCTION__, operation, result); \
    }

#define SAVE_VAL(section, key, option) \
    SetValue(ini, section, key, option)

#define LOAD_VAL(section, key, option) \
    option = GetValue(ini, section, key, option)

CScriptSettings::CScriptSettings(std::string settingsFile)
    : mSettingsFile(std::move(settingsFile)) {

}

void CScriptSettings::Load() {
    CSimpleIniA ini;
    ini.SetUnicode();
    SI_Error result = ini.LoadFile(mSettingsFile.c_str());
    CHECK_LOG_SI_ERROR(result, "load");

    LOAD_VAL("Main", "UpdateIntervalMs", Main.UpdateIntervalMs);

    LOAD_VAL("Patch", "PatchMode", Patch.PatchMode);
    LOAD_VAL("Patch", "UnpatchDistance", Patch.UnpatchDistance);
}

void CScriptSettings::Save() {
    CSimpleIniA ini;
    ini.SetUnicode();
    SI_Error result = ini.LoadFile(mSettingsFile.c_str());
    CHECK_LOG_SI_ERROR(result, "load");

    SAVE_VAL("Main", "UpdateIntervalMs", Main.UpdateIntervalMs);

    SAVE_VAL("Patch", "PatchMode", Patch.PatchMode);
    SAVE_VAL("Patch", "UnpatchDistance", Patch.UnpatchDistance);

    result = ini.SaveFile(mSettingsFile.c_str());
    CHECK_LOG_SI_ERROR(result, "save");
}
