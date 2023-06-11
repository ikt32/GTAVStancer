#pragma once
#include <string>
#include <vector>

class CScriptSettings
{
public:
    CScriptSettings(std::string settingsFile);

    void Load();
    void Save();

    struct {
        int UpdateIntervalMs = 5000;
    } Main;

    struct {
        // When patching should be enabled or disabled
        // -1: Never patch
        // 0: Always patch (0.3.0 behavior)
        // 1: Player compatibility: Unpatch when the player-occupied vehicle is
        //    a lowrider or a special vehicle.
        // 2: World compatibility: Unpatch when any vehicle within x meters of the
        //    player is a lowrider or a special vehicle.
        int PatchMode = 0;
        float UnpatchDistance = 50.0f;
    } Patch;

private:
    std::string mSettingsFile;
};
