#pragma once

#include "inc\natives.h"
#include "inc\types.h"
#include "inc\enums.h"
#include "inc\main.h"

#include <string>
#include "presets.h"
#include <vector>

#define DISPLAY_VERSION "v0.2.2"

const std::string modDir  = "\\VStancer";

void ScriptMain();
void unloadPatch();

void update_menu();
void patchHeightReset();
void savePreset(bool asPreset, std::string presetName);
void ultraSlam(Vehicle handle, float frontCamber, float rearCamber, float frontTrackWidth, float rearTrackWidth, float frontHeight, float rearHeight);
void deletePreset(Preset preset, const std::vector<Preset> &fromWhich);
void getStats(Vehicle handle);
void oldSlam(Vehicle vehicle, int slamLevel);
void applyPreset(std::vector<Preset>::value_type preset);
