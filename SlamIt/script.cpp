#include "script.h"
#include "keyboard.h"
#include "Util/Util.hpp"

#include "../../GTAVManualTransmission/Gears/Memory/VehicleExtensions.hpp"
#include "../../GTAVManualTransmission/Gears/Memory/NativeMemory.hpp"
#include "../../GTAVManualTransmission/Gears/Util/simpleini/SimpleIni.h"

#include "Menu/controls.h"
#include "presets.h"
#include "settings.h"
#include "Menu/MenuClass.h"
#include "Util/Paths.h"
#include "Util/Logger.hpp"

Menu menu;

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

MenuControls controls;
Settings settings;

// The current values, updated by getStats
float g_frontCamber;
float g_frontTrackWidth;
float g_frontHeight;

float g_rearCamber;
float g_rearTrackWidth;
float g_rearHeight;

float g_visualHeight;
int slamLevel = 0;

bool autoApplied = false;

const int offsetCamber = 0x008;
const int offsetCamberInv = 0x010;
const int offsetTrackWidth = 0x030;
const int offsetHeight = 0x038; // affected by hydraulics! 0x028 also.

// Keep track of menu highlight for control disable while typing
bool showOnlyCompatible = false;

// TODO: Patching stuff
// Assembly shit
// GTA5.exe + F1023B - F3 0F11 43 28	- movss[rbx + 28], xmm0
// GTA5.exe + F10240 - F3 44 0F11 63 20 - movss[rbx + 20], xmm12
// GTA5.exe + F10246 - F3 44 0F11 4B 24 - movss[rbx + 24], xmm9
// GTA5.exe + F1024C - F3 0F11 73 30	- movss[rbx + 30], xmm6 // track width
// GTA5.exe + F10251 - F3 0F11 5B 34	- movss[rbx + 34], xmm3 // dunno but it seems to be unique
// GTA5.exe + F10256 - F3 0F11 63 38	- movss[rbx + 38], xmm4 // height

MemoryAccess mem;
uintptr_t heightSetInstrPointerAddress = 0;
int attempts = 0;
const int maxAttempts = 5;
byte origInstr[5] = { 0xF3, 0x0F, 0x11, 0x63, 0x38 };

uintptr_t patchShit() {
	uintptr_t address = NULL;
	if (heightSetInstrPointerAddress != NULL) {
		address = heightSetInstrPointerAddress;
	} else if (attempts < maxAttempts) {
		address = mem.FindPattern("\xF3\x0F\x11\x63\x38", "xxx?x");
		heightSetInstrPointerAddress = address;
	}
	if (address != NULL) {
		byte newInstr[5] = { 0x90, 0x90, 0x90, 0x90, 0x90 };

		memcpy(origInstr, reinterpret_cast<void*>(address), 6);	// save whole orig instruction
		memcpy(reinterpret_cast<void*>(address), newInstr, 6);	// NOP that shit
		attempts = 0;
		return address;
	}
	attempts++;
	return 0;
}

uintptr_t blackMagicToGetWheelPtrFromBytecode() {

	return 0;
}

// this thing should probably be run in another thread by its own to be FAST?
void checkEachThing(Vehicle handle) {
	auto wheelPtr = ext.GetWheelsPtr(handle);  // pointer to wheel pointers
	auto numWheels = ext.GetNumWheels(handle);

	for (auto i = 0; i < numWheels; i++) {
		auto wheelAddr = *reinterpret_cast<uint64_t *>(wheelPtr + 0x008 * i);
		if (blackMagicToGetWheelPtrFromBytecode() == wheelAddr) {
			patchShit();
		}
	}
}

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
	settings.ReadSettings(&controls, &menu);
	menu.LoadMenuTheme(std::wstring(settingsMenuFile.begin(), settingsMenuFile.end()).c_str());
	logger.Write("Settings read");

	// Depending on how crappy the XML is this shit might crash and burn.
	try {
		presets = settings.ReadPresets(presetCarsFile);
		saved = settings.ReadPresets(savedCarsFile);
	}
	catch (...) {
		showSubtitle("Unknown XML read error!");
		logger.Write("Unknown XML read error!");
	}
	logger.Write("Initialization finished");

}

void savePreset(bool asPreset, std::string presetName) {
	std::string name = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(model);
	std::string plate;
	struct Preset::WheelInfo front = { g_frontCamber, g_frontTrackWidth, g_frontHeight};
	struct Preset::WheelInfo rear = { g_rearCamber, g_rearTrackWidth, g_rearHeight };

	if (asPreset) {
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
		settings.OverwritePreset(Preset(front, rear, name, plate), asPreset ? presetCarsFile : savedCarsFile);
		showNotification(asPreset ? "Updated preset" : "Updated car", &prevNotification);
	}
	else {
		try {
			settings.AppendPreset(Preset(front, rear, name, plate), asPreset ? presetCarsFile : savedCarsFile);
			showNotification(asPreset ? "Saved new preset" : "Saved new car", &prevNotification);
		}
		catch (std::runtime_error ex) {
			logger.Write(ex.what());
			logger.Write("Saving of " + plate + " to " + (asPreset ? presetCarsFile : savedCarsFile) + " failed!");
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

void choosePresetMenu(std::string title, std::vector<Preset> whichPresets) {
	menu.Title(title);
	std::string currentName = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(model);
	std::string compatibleString = "Show only " + currentName;
	menu.BoolOption(compatibleString, &showOnlyCompatible);
	for (auto preset : whichPresets) {
		if (showOnlyCompatible) {
			if (preset.Name() != currentName) {
				continue;
			}
		}
		std::string label = preset.Name() + " " + preset.Plate();
		std::vector<std::string> info;
		info.push_back("Press RIGHT to delete preset");
		info.push_back("Front Camber\t\t" + std::to_string(preset.Front.Camber));
		info.push_back("Front Track width\t" + std::to_string(preset.Front.TrackWidth));
		info.push_back("Front Height\t\t" + std::to_string(preset.Front.Height));
		info.push_back("Rear  Camber\t\t" + std::to_string(preset.Rear.Camber));
		info.push_back("Rear  Track width\t" + std::to_string(preset.Rear.TrackWidth));
		info.push_back("Rear  Height\t\t" + std::to_string(preset.Rear.Height));
		if (menu.OptionPlus(label, info, nullptr, std::bind(deletePreset, preset, whichPresets), nullptr)) {
			ultraSlam(vehicle,
			          preset.Front.Camber,
			          preset.Rear.Camber,
			          preset.Front.TrackWidth,
			          preset.Rear.TrackWidth,
			          preset.Front.Height,
			          preset.Rear.Height);
			getStats(vehicle);
			showNotification("Applied preset!", &prevNotification);
		}
	}
}

/*
 * I got the menu class from "Unknown Modder", he got it from SudoMod.
 */
void update_menu() {
	menu.CheckKeys(&controls, std::bind(init), nullptr);

	if (menu.CurrentMenu("mainmenu")) {
		menu.Title("VStancer"); // TODO: Less sucky names

		if (menu.BoolOption("Enable mod", &settings.enableMod)) { settings.SaveSettings(); }
		menu.MenuOption("Suspension menu", "suspensionmenu");
		menu.MenuOption("Load a preset", "presetmenu");
		menu.MenuOption("List car configs", "carsmenu");
		if (menu.Option("Save as car")) {
			savePreset(false,"");
		}
		if (menu.Option("Save as preset")) {
			savePreset(true , "");
		}
		if (menu.BoolOption("Auto apply cars", &settings.autoApply)) { settings.SaveSettings(); }
		menu.MenuOption("Other stuff", "othermenu");
	}

	if (menu.CurrentMenu("suspensionmenu")) {
		menu.Title("Suspension menu");

		menu.FloatOption( "Front Camber\t\t",	&g_frontCamber, -2.0f, 2.0f, 0.01f);
		menu.FloatOption( "Front Track Width", &g_frontTrackWidth, -2.0f, 2.0f, 0.01f);
		menu.FloatOption( "Front Height\t\t",   &g_frontHeight, -2.0f, 2.0f, 0.01f);
							 											   
		menu.FloatOption( "Rear  Camber\t\t",   &g_rearCamber, -2.0f, 2.0f, 0.01f); 
		menu.FloatOption( "Rear  Track Width", &g_rearTrackWidth, -2.0f, 2.0f, 0.01f);
		menu.FloatOption( "Rear  Height\t\t",   &g_rearHeight, -2.0f, 2.0f, 0.01f); 
	}

	if (menu.CurrentMenu("presetmenu")) {
		choosePresetMenu("Load preset", presets);
	}

	if (menu.CurrentMenu("carsmenu")) {
		choosePresetMenu("Car overview", saved);
	}

	if (menu.CurrentMenu("othermenu")) {
		menu.Title("Other options");

		if (menu.IntOption("Slam level (SlamIt)", &slamLevel, 0, 2, 1)) {
			oldSlam(vehicle, slamLevel);
			CONTROLS::_SET_CONTROL_NORMAL(0, ControlVehicleAccelerate, 0.3f);
		}
		if (menu.FloatOption("Visual Height (LSC)", &g_visualHeight, -0.5f, 0.5f, 0.01f)) {
			ext.SetVisualHeight(vehicle, g_visualHeight);
		}
	}
	menu.EndMenu();
}

void update_game() {
	player = PLAYER::PLAYER_ID();
	playerPed = PLAYER::PLAYER_PED_ID();

	// check if player ped exists and control is on (e.g. not in a cutscene)
	if (!ENTITY::DOES_ENTITY_EXIST(playerPed) || !PLAYER::IS_PLAYER_CONTROL_ON(player)) {
		menu.CloseMenu();
		return;
	}

	// check for player ped death and player arrest
	if (ENTITY::IS_ENTITY_DEAD(playerPed) || PLAYER::IS_PLAYER_BEING_ARRESTED(player, TRUE)) {
		menu.CloseMenu();
		return;
	}

	vehicle = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);

	if (!ENTITY::DOES_ENTITY_EXIST(vehicle)) {
		prevVehicle = 0;
		autoApplied = false;
		menu.CloseMenu();
		return;
	}

	model = ENTITY::GET_ENTITY_MODEL(vehicle);
	if (!VEHICLE::IS_THIS_MODEL_A_CAR(model) && !VEHICLE::IS_THIS_MODEL_A_QUADBIKE(model)) {
		menu.CloseMenu();
		return;
	}

	if (prevVehicle != vehicle) {
		ext.ClearAddress();
		ext.GetAddress(vehicle);
		getStats(vehicle);
		prevVehicle = vehicle;
		autoApplied = false;
		menu.CloseMenu();
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
				ultraSlam(vehicle, preset.Front.Camber, preset.Rear.Camber, preset.Front.TrackWidth, preset.Rear.TrackWidth, preset.Front.Height, preset.Rear.Height);
				autoApplied = true;
				getStats(vehicle);
				showNotification("Applied preset automatically!", &prevNotification);
			}
		}
	}

	ultraSlam(vehicle, g_frontCamber, g_rearCamber, g_frontTrackWidth, g_rearTrackWidth, g_frontHeight, g_rearHeight);
}

void main() {
	logger.Write("Script started");

	settingsGeneralFile = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\settings_general.ini";
	settingsMenuFile = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\settings_menu.ini";
	savedCarsFile = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\car_preset.xml";
	presetCarsFile = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\car_saved.xml";
	settings.SetFiles(settingsGeneralFile, settingsMenuFile);

	logger.Write("Loading " + settingsGeneralFile);
	logger.Write("Loading " + settingsMenuFile);
	logger.Write("Loading " + savedCarsFile);
	logger.Write("Loading " + presetCarsFile);

	init();
	while (true) {
		update_game();
		WAIT(0);
	}
}

void ScriptMain() {
	srand(GetTickCount());
	main();
}
