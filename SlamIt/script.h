#pragma once

#include "presets.h"

#include "inc\natives.h"
#include "inc\types.h"
#include "inc\enums.h"
#include "inc\main.h"

#include <string>
#include <vector>

void ScriptMain();

void update_menu();
void savePreset(bool asPreset, std::string presetName);
void ultraSlam(Vehicle handle, float frontCamber, float rearCamber, float frontTrackWidth, float rearTrackWidth, float frontHeight, float rearHeight);
void deletePreset(Preset preset, const std::vector<Preset> &fromWhich);
void getStats(Vehicle handle);
void oldSlam(Vehicle vehicle, int slamLevel);
void applyPreset(const Preset& preset);
