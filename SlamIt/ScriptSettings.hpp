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
        bool EnableNPC = false;
    } Main;

private:
    std::string mSettingsFile;
};
