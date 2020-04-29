#pragma once
#include "presets.h"
#include "tinyxml2/tinyxml2.h"
#include <string>
#include <vector>

namespace NativeMenu {
    class MenuControls;
    class Menu;
}
class Settings
{
public:
    Settings();
    ~Settings();
    void ReadSettings();
    void SaveSettings();
    void SetFiles(const std::string &general);

    std::vector<Preset> ReadPresets(const std::string &fileName);
    void insertPreset(Preset preset, tinyxml2::XMLDocument &doc, tinyxml2::XMLNode *pRoot);
    void AppendPreset(Preset preset, const std::string &fileName);
    bool OverwritePreset(Preset preset, const std::string &fileName);
    bool DeletePreset(Preset preset, const std::string &fileName);
    bool autoApply = false;
    bool enableMod = false;
    bool enableHeight = false;
private:
    std::string settingsGeneralFile;
    std::string settingsMenuFile;
};
