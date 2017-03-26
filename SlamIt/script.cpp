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
double frontDistance;
float  frontHeight;

float  rearCamber;
double rearDistance;
float  rearHeight;

auto offsetCamber = 0x008;
auto offsetinvCamber = 0x010;
auto offsetDistance = 0x02C;
auto offsetHeight = 0x038;

void getStats(Vehicle handle) {
	auto numWheels = ext.GetNumWheels(handle);
	if (numWheels < 4)
		return;

	auto wheelPtr = ext.GetWheelsPtr(handle);  // pointer to wheel pointers
	auto wheelAddr0 = *reinterpret_cast<uint64_t *>(wheelPtr + 0x008 * 0);
	frontCamber =	*reinterpret_cast<const float *>(wheelAddr0 + offsetCamber);
	frontDistance =	-*reinterpret_cast<const double *>(wheelAddr0 + offsetDistance);
	frontHeight =	*reinterpret_cast<const float *>(wheelAddr0 + offsetHeight);

	auto wheelAddr2 = *reinterpret_cast<uint64_t *>(wheelPtr + 0x008 * 2);
	rearCamber =	*reinterpret_cast<const float *>(wheelAddr2 + offsetCamber);
	rearDistance =	-*reinterpret_cast<const double *>(wheelAddr2 + offsetDistance);
	rearHeight =	*reinterpret_cast<const float *>(wheelAddr2 + offsetHeight);

}

void ultraSlam(Vehicle handle, float camberFront, float camberRear, double distanceFront, double distanceRear, float heightFront, float heightRear) {
	auto numWheels = ext.GetNumWheels(handle);
	if (numWheels < 4)
		return;

	auto wheelPtr = ext.GetWheelsPtr(handle);  // pointer to wheel pointers


	for (auto i = 0; i < numWheels; i++) {
		float camber;
		double distance;
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
		*reinterpret_cast<double *>(wheelAddr + offsetDistance) = -distance * flip;
		*reinterpret_cast<float *>(wheelAddr + offsetHeight) = height;
	}
}
void update_menu() {
	Menu::checkKeys();

	// Has to look for mainmenu otherwise the code fails due to me setting menu to mainmenu on Keypress
	if (Menu::currentMenu("mainmenu")) {
		Menu::Title("Get Low");

		Menu::MenuOption("Suspension menu", "suspensionmenu");
		Menu::MenuOption("Load preset", "presetmenu");
		Menu::Option("Save as car");
		Menu::Option("Save as preset");
		Menu::BoolOption("Auto apply cars", &settings.autoApply);
		Menu::BoolOption("Enable stancing", &settings.enableMod);
	}

	if (Menu::currentMenu("suspensionmenu")) {
		Menu::Title("Suspension menu");

		Menu::FloatOption(  "Front Camber",	  &frontCamber,   -2.0f, 2.0f, 0.01f);
		Menu::DoubleOption( "Front Distance", &frontDistance, -1.0,  1.0,  0.00005);
		Menu::FloatOption(  "Front Height",   &frontHeight,   -2.0f, 2.0f, 0.01f);
							 											   
		Menu::FloatOption(  "Rear Camber",    &rearCamber,    -2.0f, 2.0f, 0.01f); 
		Menu::DoubleOption( "Rear Distance",  &rearDistance,  -1.0,  1.0,  0.00005); 
		Menu::FloatOption(  "Rear Height",    &rearHeight,    -2.0f, 2.0f, 0.01f); 
	}

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

	auto model = ENTITY::GET_ENTITY_MODEL(vehicle);
	if (!VEHICLE::IS_THIS_MODEL_A_CAR(model) && !VEHICLE::IS_THIS_MODEL_A_QUADBIKE(model))
		return;

	if (prevVehicle != vehicle) {
		ext.ClearAddress();
		ext.GetAddress(vehicle);
		getStats(vehicle);
		prevVehicle = vehicle;
		return;
	}

	/*if (!settings.enableMod)
		return;*/

	ultraSlam(vehicle, frontCamber, rearCamber, frontDistance, rearDistance, frontHeight, rearHeight);
	//getStats(vehicle);
}

void init() {
	settings.ReadSettings(&controls);
	// Depending on how crappy the XML is this shit might crash and burn.
	presets = settings.ReadPresets(presetsFile);
	saved = settings.ReadPresets(savedCarsFile);

	Menu::LoadMenuTheme(menuStyleLocation);

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
