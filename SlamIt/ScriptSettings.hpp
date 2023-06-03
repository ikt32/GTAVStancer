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

private:
    std::string mSettingsFile;
};
