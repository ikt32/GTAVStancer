#pragma once
#include "controls.h"
#include <string>
#include "presets.h"
#include <vector>

class Settings
{
public:
	Settings(const std::string &settingsFile);
	~Settings();
	void ReadSettings(Controls *control);
	void SaveSettings();

	std::vector<Preset> ReadPresets(const std::string &fileName);
	void AppendPreset(Preset preset, const std::string &fileName);
	bool OverwritePreset(Preset preset, const std::string &fileName);
	bool DeletePreset(Preset preset, const std::string &fileName);
	bool autoApply = false;
	bool enableMod = false;
private:
	std::string settingsFile;
};
