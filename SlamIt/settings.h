#pragma once
#include <string>
#include "presets.h"
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
	void AppendPreset(Preset preset, const std::string &fileName);
	bool OverwritePreset(Preset preset, const std::string &fileName);
	bool DeletePreset(Preset preset, const std::string &fileName);
	bool autoApply = false;
	bool enableMod = false;
private:
	std::string settingsGeneralFile;
	std::string settingsMenuFile;
};
