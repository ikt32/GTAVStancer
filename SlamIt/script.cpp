#include "script.h"

#include <sstream>
#include <simpleini/SimpleIni.h>
#include <menu.h>

#include "Patching/Patching.h"

#include "Util/Paths.h"
#include "Util/Util.hpp"
#include "Util/Logger.hpp"

#include "presets.h"
#include "settings.h"

#include "../../GTAVManualTransmission/Gears/Memory/VehicleExtensions.hpp"
#include "Offsets.h"

NativeMenu::Menu menu;

std::string settingsGeneralFile;
std::string settingsMenuFile;
std::string savedCarsFile;
std::string presetCarsFile;

Hash model;
Vehicle vehicle;
Vehicle prevVehicle;
VehicleExtensions ext;
Player player;
Ped playerPed;

int prevNotification;

std::vector<Preset> presets;
std::vector<Preset> saved;

Settings settings;

// The current values, updated by getStats
// should be continuously applied
float g_frontCamber;
float g_frontTrackWidth;
float g_frontHeight;

float g_rearCamber;
float g_rearTrackWidth;
float g_rearHeight;

float g_visualHeight;

int slamLevel = 0;

bool autoApplied = false;

// Keep track of menu highlight for control disable while typing
bool showOnlyCompatible = false;

/*
 *  Update wheels info, not sure if I should move this into vehExt.
 *  It's not really useful info aside from damage.
 *  Only the left front and and left rear wheels are used atm.
 */
void getStats(Vehicle handle) {
	auto numWheels = ext.GetNumWheels(handle);
	if (numWheels < 4)
		return;

	auto wheelPtr = ext.GetWheelsPtr(handle);
	auto wheelAddr0 =	*reinterpret_cast< uint64_t *    >(wheelPtr + 0x008 * 0);
	g_frontCamber =		*reinterpret_cast< const float * >(wheelAddr0 + offsetCamber);
	g_frontTrackWidth =	   -*reinterpret_cast< const float * >(wheelAddr0 + offsetTrackWidth);
	g_frontHeight =		*reinterpret_cast< const float * >(wheelAddr0 + offsetHeight);

	auto wheelAddr2 =	*reinterpret_cast< uint64_t *    >(wheelPtr + 0x008 * 2);
	g_rearCamber =		*reinterpret_cast< const float * >(wheelAddr2 + offsetCamber);
	g_rearTrackWidth =	   -*reinterpret_cast< const float * >(wheelAddr2 + offsetTrackWidth);
	g_rearHeight =		*reinterpret_cast< const float * >(wheelAddr2 + offsetHeight);

	g_visualHeight = ext.GetVisualHeight(handle);
}

/*
 * Write new values. getStats should be called after running this with fresh
 * new values. Otherwise this should be called constantly unless I get to patching stuff.
 * Can't camber trikes and stuff for now
 */
void ultraSlam(Vehicle handle, float frontCamber, float rearCamber, float frontTrackWidth, float rearTrackWidth, float frontHeight, float rearHeight) {
	auto numWheels = ext.GetNumWheels(handle);
	if (numWheels < 4)
		return;

	auto wheelPtr = ext.GetWheelsPtr(handle);

	for (auto i = 0; i < numWheels; i++) {
		float camber;
		float trackWidth;
		float height;

		if (i == 0 || i ==  1) {
			camber = frontCamber;
			trackWidth = frontTrackWidth;
			height = frontHeight;
		} else {
			camber = rearCamber;
			trackWidth = rearTrackWidth;
			height = rearHeight;
		}

		float flip = i % 2 == 0 ? 1.0f : -1.0f; // cuz the wheels on the other side
		auto wheelAddr = *reinterpret_cast<uint64_t *>(wheelPtr + 0x008 * i);
		*reinterpret_cast<float *>(wheelAddr + offsetCamber) = camber * flip;
		*reinterpret_cast<float *>(wheelAddr + offsetCamberInv) = -camber * flip;
		*reinterpret_cast<float *>(wheelAddr + offsetTrackWidth) = -trackWidth * flip;
		*reinterpret_cast<float *>(wheelAddr + offsetHeight) = height;
	}
}

/*
 * Old "Damage the wheels" thing:
 */
void oldSlam(Vehicle vehicle, int slamLevel) {
	switch (slamLevel) {
		case (2):
			ext.SetWheelsHealth(vehicle, 0.0f);
			break;
		case (1):
			ext.SetWheelsHealth(vehicle, 400.0f);
			break;
		default:
		case (0):
			ext.SetWheelsHealth(vehicle, 1000.0f);
			break;
	}
}

void init() {
    ext.initOffsets();
	settings.ReadSettings();
	menu.ReadSettings();
	logger.Write(INFO, "Settings read");

	// Depending on how crappy the XML is this shit might crash and burn.
	try {
		presets = settings.ReadPresets(presetCarsFile);
		saved = settings.ReadPresets(savedCarsFile);
	}
	catch (...) {
		showSubtitle("Unknown XML read error!");
		logger.Write(ERROR, "Unknown XML read error!");
	}
	logger.Write(INFO, "Initialization finished");

}

void savePreset(bool asPreset, std::string presetName) {
	std::string name = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(model);
	std::string plate;
	Preset::Suspension front { g_frontCamber, g_frontTrackWidth, g_frontHeight};
	Preset::Suspension rear { g_rearCamber, g_rearTrackWidth, g_rearHeight };

    auto wheels = ext.GetWheelPtrs(vehicle);

    Preset::WheelPhys frontWheels { 
        *reinterpret_cast<float *>(wheels[0] + offTyreRadius),
        *reinterpret_cast<float *>(wheels[0] + offTyreWidth)
    };
    Preset::WheelPhys rearWheels {
        *reinterpret_cast<float *>(wheels[2] + offTyreRadius),
        *reinterpret_cast<float *>(wheels[2] + offTyreWidth)
    };

    Preset::WheelVis visualSize { 0.0f, 0.0f, -1, -1};

    auto CVeh_0x48 = *(uint64_t *)(ext.GetAddress(vehicle) + 0x48);
    auto CVeh_0x48_0x370 = *(uint64_t *)(CVeh_0x48 + 0x370);
    if (CVeh_0x48_0x370 != 0) {
        visualSize = {
            *(float *)(CVeh_0x48_0x370 + 0x8),
            *(float *)(CVeh_0x48_0x370 + 0xB80),
            VEHICLE::GET_VEHICLE_WHEEL_TYPE(vehicle),
            VEHICLE::GET_VEHICLE_MOD(vehicle, VehicleModFrontWheels)
        };
    }

	if (asPreset) {
        WAIT(32);
		GAMEPLAY::DISPLAY_ONSCREEN_KEYBOARD(true, "Preset Name", "", "", "", "", "", 127);
		while (GAMEPLAY::UPDATE_ONSCREEN_KEYBOARD() == 0) WAIT(0);
		if (!GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT()) {
			showNotification("Cancelled save");
			return;
		}
		presetName = GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT();
		
	}

	if (asPreset) {
		if (presetName.empty()) {
			plate = Preset::ReservedPlate();
		} else {
			plate = presetName;
		}
	} else {
		plate = VEHICLE::GET_VEHICLE_NUMBER_PLATE_TEXT(vehicle);
	}

	bool alreadyPresent = false;

	for (auto preset : asPreset ? presets : saved) {
		if (plate == preset.Plate() &&
			name == preset.Name()) {
			alreadyPresent = true;
		}
	}

	if (alreadyPresent) {
		settings.OverwritePreset(
            Preset(front, rear, frontWheels, rearWheels, visualSize, g_visualHeight, name, plate),
            asPreset ? presetCarsFile : savedCarsFile);
		showNotification(asPreset ? "Updated preset" : "Updated car", &prevNotification);
	}
	else {
		try {
			settings.AppendPreset(
                Preset(front, rear, frontWheels, rearWheels, visualSize, g_visualHeight, name, plate),
                asPreset ? presetCarsFile : savedCarsFile);
			showNotification(asPreset ? "Saved new preset" : "Saved new car", &prevNotification);
		}
		catch (std::runtime_error ex) {
			logger.Write(ERROR, ex.what());
			logger.Write(ERROR, "Saving of %s to %s failed!", 
                plate.c_str(), (asPreset ? presetCarsFile : savedCarsFile).c_str());
			showNotification("Saving to xml failed!");
		}
	}
	init();
}

/*
 * Scan current configs and remove. Since I cba to scan two lists again and the caller
 * should probably know which list it is from anyway that list is passed.
 */
void deletePreset(Preset preset, const std::vector<Preset> &fromWhich) {
	std::string fromFile;
	std::string message = "Couldn't find " + preset.Name() + " " + preset.Plate() + " :(";
	if (fromWhich == presets) {
		fromFile = presetCarsFile;
	}
	if (fromWhich == saved) {
		fromFile = savedCarsFile;
	}
	if (fromFile.empty()) {
		message = "File empty?";
		showNotification(message.c_str(), &prevNotification);
		return;
	}

	if (settings.DeletePreset(preset, fromFile)) {
		message = "Deleted preset " + preset.Name() + " " + preset.Plate();
		init();
	}
	showNotification(message.c_str(), &prevNotification);
}

void applyPreset(std::vector<Preset>::value_type preset) {
    ultraSlam(vehicle,
        preset.FrontSuspension.Camber,
        preset.RearSuspension.Camber,
        preset.FrontSuspension.TrackWidth,
        preset.RearSuspension.TrackWidth,
        preset.FrontSuspension.Height,
        preset.RearSuspension.Height);

    if (preset.VisualHeight != -1337.0f)
        ext.SetVisualHeight(vehicle, preset.VisualHeight);

    auto wheels = ext.GetWheelPtrs(vehicle);
    auto numWheels = ext.GetNumWheels(vehicle);
    if (preset.FrontWheels.TyreWidth != -1337.0f && preset.FrontWheels.TyreWidth != -1337.0f) {
        for (int i = 0; i < 2; i++) {
            *reinterpret_cast<float *>(wheels[i] + offTyreRadius) = preset.FrontWheels.TyreRadius;
            *reinterpret_cast<float *>(wheels[i] + offTyreWidth) = preset.FrontWheels.TyreWidth;
        }
    }

    if (preset.RearWheels.TyreWidth != -1337.0f && preset.RearWheels.TyreWidth != -1337.0f) {
        for (int i = 2; i < numWheels; i++) {
            *reinterpret_cast<float *>(wheels[i] + offTyreRadius) = preset.RearWheels.TyreRadius;
            *reinterpret_cast<float *>(wheels[i] + offTyreWidth) = preset.RearWheels.TyreWidth;
        }
    }

    if (preset.VisualSize.WheelType != -1 && preset.VisualSize.WheelIndex != -1) {
        VEHICLE::SET_VEHICLE_MOD_KIT(vehicle, 0);
        auto customtyres = VEHICLE::GET_VEHICLE_MOD_VARIATION(vehicle, VehicleModFrontWheels);
        VEHICLE::SET_VEHICLE_WHEEL_TYPE(vehicle, preset.VisualSize.WheelType);
        VEHICLE::SET_VEHICLE_MOD(vehicle, VehicleModFrontWheels, preset.VisualSize.WheelIndex, customtyres);
        WAIT(500);
        auto CVeh_0x48 = *(uint64_t *)(ext.GetAddress(vehicle) + 0x48);
        auto CVeh_0x48_0x370 = *(uint64_t *)(CVeh_0x48 + 0x370);
        if (CVeh_0x48_0x370 != 0) {
            *(float *)(CVeh_0x48_0x370 + 0x8) = preset.VisualSize.WheelSize;
            *(float *)(CVeh_0x48_0x370 + 0xB80) = preset.VisualSize.WheelWidth;
        }
        else {
            showNotification("Couldn't set visual sizes!");
        }
    }
}

void update_game() {
	player = PLAYER::PLAYER_ID();
	playerPed = PLAYER::PLAYER_PED_ID();

	if (!ENTITY::DOES_ENTITY_EXIST(playerPed) || !PLAYER::IS_PLAYER_CONTROL_ON(player) ||
		ENTITY::IS_ENTITY_DEAD(playerPed) || PLAYER::IS_PLAYER_BEING_ARRESTED(player, TRUE)) {
		return;
	}

	vehicle = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);

	if (!ENTITY::DOES_ENTITY_EXIST(vehicle)) {
		prevVehicle = 0;
		autoApplied = false;
		return;
	}

	model = ENTITY::GET_ENTITY_MODEL(vehicle);
	if (!VEHICLE::IS_THIS_MODEL_A_CAR(model) && !VEHICLE::IS_THIS_MODEL_A_QUADBIKE(model)) {
		unloadPatch();
		return;
	}

	if (prevVehicle != vehicle) {
		if (!ext.GetAddress(vehicle)) {
			return;
		}
		getStats(vehicle);
		prevVehicle = vehicle;
		autoApplied = false;
		return;
	}

	update_menu();

	if (!settings.enableMod) {
		return;
	}

	if (settings.autoApply && !autoApplied) {
		for (auto preset : saved) {
			if (VEHICLE::GET_VEHICLE_NUMBER_PLATE_TEXT(vehicle) == preset.Plate() &&
				VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(model) == preset.Name()) {
                applyPreset(preset);

				getStats(vehicle);
				showNotification("Applied preset automatically!", &prevNotification);
                autoApplied = true;
            }
		}
	}

	ultraSlam(vehicle, g_frontCamber, g_rearCamber, g_frontTrackWidth, g_rearTrackWidth, g_frontHeight, g_rearHeight);
}

void main() {
	logger.Write(INFO, "Script started");

	settingsGeneralFile = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\settings_general.ini";
	settingsMenuFile = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\settings_menu.ini";
	savedCarsFile = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\car_preset.xml";
	presetCarsFile = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\car_saved.xml";
	settings.SetFiles(settingsGeneralFile);
	menu.SetFiles(settingsMenuFile);
	menu.RegisterOnMain(std::bind(init));

	logger.Write(INFO, "Loading " + settingsGeneralFile);
	logger.Write(INFO, "Loading " + settingsMenuFile);
	logger.Write(INFO, "Loading " + savedCarsFile);
	logger.Write(INFO, "Loading " + presetCarsFile);

	init();

	if (settings.enableMod) {
		patchHeightReset();
	}

	while (true) {
		update_game();
		WAIT(0);
	}
}

void ScriptMain() {
	srand(GetTickCount());
	main();
}
