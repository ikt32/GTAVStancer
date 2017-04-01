#include "script.h"
#include "keyboard.h"
#include "../../GTAVManualTransmission/Gears/Memory/VehicleExtensions.hpp"
#include "../../GTAVManualTransmission/Gears/Util/Util.hpp"
#include "../../GTAVManualTransmission/Gears/Util/simpleini/SimpleIni.h"
#include "controls.h"
#include "presets.h"
#include "settings.h"
#include "MenuClass.h"

Menu menu;

const std::string settingsFolder = "./VStancer/";
const std::string settingsFile = settingsFolder+"Settings.ini";
const std::string savedCarsFile = settingsFolder+"SavedCars.xml";
const std::string presetsFile = settingsFolder +"Presets.xml";
LPCWSTR menuStyleLocation = L".\\VStancer\\MenuStyle.ini";

Hash model;
Vehicle vehicle;
Vehicle prevVehicle;
VehicleExtensions ext;
Player player;
Ped playerPed;

int prevNotification;

std::vector<Preset> presets;
std::vector<Preset> saved;

Controls controls;
Settings settings(settingsFile);

// The current values, updated by getStats
float frontCamber;
float frontTrackWidth;
float frontHeight;

float rearCamber;
float rearTrackWidth;
float rearHeight;

bool autoApplied = false;

auto offsetCamber = 0x008;
auto offsetCamberInv = 0x010;
auto offsetTrackWidth = 0x030;
auto offsetHeight = 0x038;

// Keep track of menu highlight for control disable while typing
std::string currentInput = "";
bool presethighlighted = false;
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
	frontCamber =		*reinterpret_cast< const float * >(wheelAddr0 + offsetCamber);
	frontTrackWidth =	   -*reinterpret_cast< const float * >(wheelAddr0 + offsetTrackWidth);
	frontHeight =		*reinterpret_cast< const float * >(wheelAddr0 + offsetHeight);

	auto wheelAddr2 =	*reinterpret_cast< uint64_t *    >(wheelPtr + 0x008 * 2);
	rearCamber =		*reinterpret_cast< const float * >(wheelAddr2 + offsetCamber);
	rearTrackWidth =	   -*reinterpret_cast< const float * >(wheelAddr2 + offsetTrackWidth);
	rearHeight =		*reinterpret_cast< const float * >(wheelAddr2 + offsetHeight);

}


/*
 * Write new values. getStats should be called after running this with fresh
 * new values. Otherwise this should be called constantly unless I get to patching stuff.
 * Can't camber trikes and stuff for now but why the *fuck* would you want to?
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

void init() {
	settings.ReadSettings(&controls);
	menu.LoadMenuTheme(menuStyleLocation);

	// Depending on how crappy the XML is this shit might crash and burn.
	try {
		presets = settings.ReadPresets(presetsFile);
		saved = settings.ReadPresets(savedCarsFile);
	}
	catch (...) {
		showNotification("Unknown read error!", &prevNotification);
	}
}

void savePreset(bool asPreset, std::string presetName) {
	std::string name = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(model);
	std::string plate;
	struct Preset::WheelInfo front = { frontCamber, frontTrackWidth, frontHeight};
	struct Preset::WheelInfo rear = { rearCamber, rearTrackWidth, rearHeight };

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
		settings.OverwritePreset(Preset(front, rear, name, plate), asPreset ? presetsFile : savedCarsFile);
		showNotification(asPreset ? "Updated preset" : "Updated car", &prevNotification);
	}
	else {
		settings.AppendPreset(Preset(front, rear, name, plate), asPreset ? presetsFile : savedCarsFile);
		showNotification(asPreset ? "Saved new preset" : "Saved new car", &prevNotification);
	}
	init();
}

/*
 * Ah, the typing input thing. Scanning ascii characters from 
 * ' ' to '~' should cover the alphanumeric range pretty well and it seems to work.
 * The ' ' isn't detected though so this is weird.
 * TODO: Fix space character not read
 */
std::string evaluateInput() {
	PLAYER::IS_PLAYER_CONTROL_ON(false);
	UI::HIDE_HUD_AND_RADAR_THIS_FRAME();
	UI::SET_PAUSE_MENU_ACTIVE(false);
	CONTROLS::DISABLE_ALL_CONTROL_ACTIONS(1);
	CONTROLS::IS_CONTROL_ENABLED(playerPed, false);

	for (char c = ' '; c < '~'; c++) {
		if (IsKeyJustUp(str2key(std::string(1, c)))) {
			currentInput += c;
		}
		if (IsKeyJustUp(str2key("DELETE"))) {
			currentInput.pop_back();
		}
		if (IsKeyJustUp(str2key("BACKSPACE"))) {
			currentInput.clear();
		}
	}
	
	return currentInput;
}

/*
 * This lil' function is just here because std::function stuff. Maybe it'll get more
 * useful later on when I cram more stuff into it. Anyhow it's run on menu exit.
 */
void clearmenustuff() {
	currentInput.clear();
}

/*
 * Scan current configs and remove. Since I cba to scan two lists again and the caller
 * should probably know which list it is from anyway that list is passed.
 */
void deletePreset(Preset preset, const std::vector<Preset> &fromWhich) {
	std::string fromFile;
	std::string message = "Couldn't find " + preset.Name() + " " + preset.Plate() + " :(";
	if (fromWhich == presets) {
		fromFile = presetsFile;
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
	menu.Title(CharAdapter(title.c_str()));
	std::string currentName = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(model);
	std::string compatibleString = "Show only " + currentName;
	menu.BoolOption(CharAdapter(compatibleString.c_str()), &showOnlyCompatible);
	for (auto preset : whichPresets) {
		if (showOnlyCompatible) {
			if (preset.Name() != currentName) {
				continue;
			}
		}
		std::string label = preset.Name() + " " + preset.Plate();
		std::vector<std::string> info;
		info.push_back("Press RIGHT to delete preset");
		info.push_back("Front Camber      " + std::to_string(preset.Front.Camber));
		info.push_back("Front Track width " + std::to_string(preset.Front.TrackWidth));
		info.push_back("Front Height      " + std::to_string(preset.Front.Height));
		info.push_back("Rear  Camber      " + std::to_string(preset.Rear.Camber));
		info.push_back("Rear  Track width " + std::to_string(preset.Rear.TrackWidth));
		info.push_back("Rear  Height      " + std::to_string(preset.Rear.Height));
		if (menu.OptionPlus(CharAdapter(label.c_str()), info, nullptr, std::bind(deletePreset, preset, whichPresets), nullptr)) {
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
	menu.CheckKeys(&controls, std::bind(init), std::bind(clearmenustuff));

	if (menu.CurrentMenu("mainmenu")) {
		menu.Title("VStancer"); // TODO: Less sucky names

		menu.MenuOption("Suspension menu", "suspensionmenu");
		menu.MenuOption("Load a preset", "presetmenu");
		menu.MenuOption("List car configs", "carsmenu");
		if (menu.Option("Save as car")) {
			savePreset(false,"");
		}
		std::vector<std::string> derp = { "Enter preset name", currentInput };
		if (menu.OptionPlus("Save as preset", derp , &presethighlighted, nullptr, nullptr)) {
			savePreset(true , derp[1]);
			currentInput.clear();
		}
		if (presethighlighted) {
			evaluateInput();
			presethighlighted = false;
		}
		if (menu.BoolOption("Auto apply cars", &settings.autoApply)) { settings.SaveSettings(); }
		if (menu.BoolOption("Enable mod",		&settings.enableMod)) { settings.SaveSettings(); }

	}

	if (menu.CurrentMenu("suspensionmenu")) {
		menu.Title("Suspension menu");

		menu.FloatOption( "Front Camber     ",	&frontCamber, -2.0f, 2.0f, 0.01f);
		menu.FloatOption( "Front Track Width", &frontTrackWidth, -2.0f, 2.0f, 0.01f);
		menu.FloatOption( "Front Height     ",   &frontHeight, -2.0f, 2.0f, 0.01f);
							 											   
		menu.FloatOption( "Rear  Camber     ",   &rearCamber, -2.0f, 2.0f, 0.01f); 
		menu.FloatOption( "Rear  Track Width", &rearTrackWidth, -2.0f, 2.0f, 0.01f);
		menu.FloatOption( "Rear  Height     ",   &rearHeight, -2.0f, 2.0f, 0.01f); 
	}

	if (menu.CurrentMenu("presetmenu")) {
		choosePresetMenu("Load preset", presets);
	}

	if (menu.CurrentMenu("carsmenu")) {
		choosePresetMenu("Car overview", saved);
	}
	menu.EndMenu();
}

void update_game() {
	player = PLAYER::PLAYER_ID();
	playerPed = PLAYER::PLAYER_PED_ID();

	// check if player ped exists and control is on (e.g. not in a cutscene)
	if (!ENTITY::DOES_ENTITY_EXIST(playerPed) || !PLAYER::IS_PLAYER_CONTROL_ON(player))
		return;

	// check for player ped death and player arrest
	if (ENTITY::IS_ENTITY_DEAD(playerPed) || PLAYER::IS_PLAYER_BEING_ARRESTED(player, TRUE))
		return;

	vehicle = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);

	if (!ENTITY::DOES_ENTITY_EXIST(vehicle)) {
		prevVehicle = 0;
		autoApplied = false;
		return;
	}

	model = ENTITY::GET_ENTITY_MODEL(vehicle);
	if (!VEHICLE::IS_THIS_MODEL_A_CAR(model) && !VEHICLE::IS_THIS_MODEL_A_QUADBIKE(model))
		return;

	if (prevVehicle != vehicle) {
		ext.ClearAddress();
		ext.GetAddress(vehicle);
		getStats(vehicle);
		prevVehicle = vehicle;
		autoApplied = false;
		return;
	}

	if (!settings.enableMod)
		return;

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

	ultraSlam(vehicle, frontCamber, rearCamber, frontTrackWidth, rearTrackWidth, frontHeight, rearHeight);
}

void main() {
	init();
	while (true) {
		update_game();
		update_menu();
		WAIT(0);
	}
}

void ScriptMain() {
	srand(GetTickCount());
	main();
}
