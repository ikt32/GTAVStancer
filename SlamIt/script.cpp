#include "script.h"
#include "keyboard.h"
#include "../../GTAVManualTransmission/Gears/Memory/VehicleExtensions.hpp"
#include "../../GTAVManualTransmission/Gears/Util/Util.hpp"
#include "../../GTAVManualTransmission/Gears/Util/simpleini/SimpleIni.h"
#include "controls.h"
#include "presets.h"
#include "settings.h"
#include "MenuClass.h"
//#include "menu.h"

//const std::vector<MenuPair> menuMainOptions = {
//	{ "Car options",		"submenu" },
//	{ "Load preset",		"submenu" },
//	{ "Save car",			"option" },
//	{ "Save preset",		"option" },
//	{ "Auto apply cars",	"option" },
//	{ "Disable mod",		"option" }
//};
//
//const std::vector<MenuPair> carOptionsOptions = {
//
//};
//
//const std::vector<MenuPair> loadPresetOptions = {
//
//};

//MenuScreen mainMenu("Main menu", menuMainOptions);

bool displayMenu = false;

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
//Menu menu;
int prevNotification;

std::vector<Preset> presets;
std::vector<Preset> saved;

Controls controls;
Settings settings(settingsFile);

float  frontCamber;
float frontDistance;
float  frontHeight;

float  rearCamber;
float rearDistance;
float  rearHeight;

bool autoApplied = false;

auto offsetCamber = 0x008;
auto offsetinvCamber = 0x010;
auto offsetDistance = 0x030;
auto offsetHeight = 0x038;

void getStats(Vehicle handle) {
	auto numWheels = ext.GetNumWheels(handle);
	if (numWheels < 4)
		return;

	auto wheelPtr = ext.GetWheelsPtr(handle);  // pointer to wheel pointers
	auto wheelAddr0 =	*reinterpret_cast< uint64_t *     >(wheelPtr + 0x008 * 0);
	frontCamber =		*reinterpret_cast< const float *  >(wheelAddr0 + offsetCamber);
	frontDistance =	   -*reinterpret_cast< const float * >(wheelAddr0 + offsetDistance);
	frontHeight =		*reinterpret_cast< const float *  >(wheelAddr0 + offsetHeight);

	auto wheelAddr2 =	*reinterpret_cast< uint64_t *     >(wheelPtr + 0x008 * 2);
	rearCamber =		*reinterpret_cast< const float *  >(wheelAddr2 + offsetCamber);
	rearDistance =	   -*reinterpret_cast< const float * >(wheelAddr2 + offsetDistance);
	rearHeight =		*reinterpret_cast< const float *  >(wheelAddr2 + offsetHeight);

}

void ultraSlam(Vehicle handle, float camberFront, float camberRear, float distanceFront, float distanceRear, float heightFront, float heightRear) {
	auto numWheels = ext.GetNumWheels(handle);
	if (numWheels < 4)
		return;

	auto wheelPtr = ext.GetWheelsPtr(handle);  // pointer to wheel pointers


	for (auto i = 0; i < numWheels; i++) {
		float camber;
		float distance;
		float height;
		if (i == 0 || i ==  1) {
			camber = camberFront;
			distance = distanceFront;
			height = heightFront;
		} else {
			camber = camberRear;
			distance = distanceRear;
			height = heightRear;
		}

		float flip = i % 2 == 0 ? 1.0f : -1.0f; // cuz the wheels on the other side
		auto wheelAddr = *reinterpret_cast<uint64_t *>(wheelPtr + 0x008 * i);
		*reinterpret_cast<float *>(wheelAddr + offsetCamber) = camber * flip;
		*reinterpret_cast<float *>(wheelAddr + offsetinvCamber) = -camber * flip;
		*reinterpret_cast<float *>(wheelAddr + offsetDistance) = -distance * flip;
		*reinterpret_cast<float *>(wheelAddr + offsetHeight) = height;
	}
}

void init() {
	settings.ReadSettings(&controls);
	Menu::LoadMenuTheme(menuStyleLocation);

	// Depending on how crappy the XML is this shit might crash and burn.
	try {
		presets = settings.ReadPresets(presetsFile);
		saved = settings.ReadPresets(savedCarsFile);
	}
	catch (...) {
		prevNotification = showNotification("Unknown read error!", prevNotification);
	}
}

void savePreset(bool asPreset) {
	std::string name = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(model);
	std::string plate = VEHICLE::GET_VEHICLE_NUMBER_PLATE_TEXT(vehicle);
	struct Preset::WheelInfo front = { frontCamber, frontDistance, frontHeight};
	struct Preset::WheelInfo rear = { rearCamber, rearDistance, rearHeight };

	if (asPreset) {
		plate = Preset::ReservedPlate();
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
		prevNotification = showNotification(asPreset ? "Saved preset" : "Saved car", prevNotification);
	}
	else {
		settings.AppendPreset(Preset(front, rear, name, plate), asPreset ? presetsFile : savedCarsFile);
		prevNotification = showNotification(asPreset ? "Added preset" : "Added car", prevNotification);
	}
	init();
}

void update_menu() {
	Menu::checkKeys(&controls, init, nullptr);

	// Has to look for mainmenu otherwise the code fails due to me setting menu to mainmenu on Keypress
	if (Menu::currentMenu("mainmenu")) {
		Menu::Title("Slam It v2"); // TODO: Less sucky names

		Menu::MenuOption("Suspension menu", "suspensionmenu");
		Menu::MenuOption("Load a preset", "presetmenu");
		Menu::MenuOption("List car configs", "carsmenu");
		if (Menu::Option("Save as car")) { savePreset(false); }
		if (Menu::Option("Save as preset")) { savePreset(true); }
		if (Menu::BoolOption("Auto apply cars", &settings.autoApply)) { settings.SaveSettings(); }
		if (Menu::BoolOption("Enable mod",		&settings.enableMod)) { settings.SaveSettings(); }
	}

	if (Menu::currentMenu("suspensionmenu")) {
		Menu::Title("Suspension menu");

		Menu::FloatOption( "Front Camber",	  &frontCamber,   -2.0f, 2.0f, 0.01f);
		Menu::FloatOption( "Front Distance", &frontDistance,  -2.0f, 2.0f,  0.01f);
		Menu::FloatOption( "Front Height",   &frontHeight,   -2.0f, 2.0f, 0.01f);
							 											   
		Menu::FloatOption( "Rear  Camber",    &rearCamber,    -2.0f, 2.0f, 0.01f); 
		Menu::FloatOption( "Rear  Distance",  &rearDistance,   -2.0f, 2.0f,  0.01f);
		Menu::FloatOption( "Rear  Height",    &rearHeight,    -2.0f, 2.0f, 0.01f); 
	}

	// Unique name (1 per car model)
	if (Menu::currentMenu("presetmenu")) {
		Menu::Title("Load preset");
		for (auto preset : presets) {
			std::string label = preset.Name();
			char * label_ = (char *)label.c_str();
			Menu::MenuOption(label_, label_);
		}
	}

	// Unique name + plate
	if (Menu::currentMenu("carsmenu")) {
		Menu::Title("Car overview");
		for (auto preset : saved) {
			std::string label = preset.Name() + " " + preset.Plate();
			char * label_ = (char *)label.c_str();
			Menu::MenuOption(label_, label_);
		}
	}

	for (auto preset : presets) {
		std::string label = preset.Name();
		char * label_ = (char *)label.c_str();
		if (Menu::currentMenu(label_)) {
			Menu::Title(label_);
			std::string fc = "Front Camber    " + std::to_string(preset.Front.Camber);
			std::string fd = "Front Distance  " + std::to_string(preset.Front.Distance);
			std::string fh = "Front Height    " + std::to_string(preset.Front.Height);
			std::string rc = "Rear  Camber    " + std::to_string(preset.Rear.Camber);
			std::string rd = "Rear  Distance  " + std::to_string(preset.Rear.Distance);
			std::string rh = "Rear  Height    " + std::to_string(preset.Rear.Height);

			Menu::Option((char*)fc.c_str());
			Menu::Option((char*)fd.c_str());
			Menu::Option((char*)fh.c_str());
			Menu::Option((char*)rc.c_str());
			Menu::Option((char*)rd.c_str());
			Menu::Option((char*)rh.c_str());

		}
	}

	for (auto preset : saved) {
		std::string label = preset.Name() + " " + preset.Plate();
		char * label_ = (char *)label.c_str();
		if (Menu::currentMenu(label_)) {
			Menu::Title(label_);
			std::string fc = "Front Camber    " + std::to_string(preset.Front.Camber);
			std::string fd = "Front Distance  " + std::to_string(preset.Front.Distance);
			std::string fh = "Front Height    " + std::to_string(preset.Front.Height);
			std::string rc = "Rear  Camber    " + std::to_string(preset.Rear.Camber);
			std::string rd = "Rear  Distance  " + std::to_string(preset.Rear.Distance);
			std::string rh = "Rear  Height    " + std::to_string(preset.Rear.Height);

			Menu::Option((char*)fc.c_str());
			Menu::Option((char*)fd.c_str());
			Menu::Option((char*)fh.c_str());
			Menu::Option((char*)rc.c_str());
			Menu::Option((char*)rd.c_str());
			Menu::Option((char*)rh.c_str());
		}
	}
	showText(0.2, 0.2, 1.0, Menu::getActualMenu());
	Menu::endMenu();
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

	if (!ENTITY::DOES_ENTITY_EXIST(vehicle))
		return;

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
				ultraSlam(vehicle, preset.Front.Camber, preset.Rear.Camber, preset.Front.Distance, preset.Rear.Distance, preset.Front.Height, preset.Rear.Height);
				autoApplied = true;
				getStats(vehicle);
			}
		}
	}

	ultraSlam(vehicle, frontCamber, rearCamber, frontDistance, rearDistance, frontHeight, rearHeight);
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
