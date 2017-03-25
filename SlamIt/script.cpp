#include "script.h"
#include "keyboard.h"
#include <sstream>
#include <iomanip>
#include "../../GTAVManualTransmission/Gears/Memory/VehicleExtensions.hpp"
#include "../../GTAVManualTransmission/Gears/Util/Util.hpp"
#include "../../GTAVManualTransmission/Gears/Util/simpleini/SimpleIni.h"
#include "controls.h"
#include "presets.h"
#include "settings.h"

const std::vector<std::string> menuMainOptions = {
	"Car options",
	"Save car",
	"Save preset",
	"Load preset",
	"Auto apply cars",
	"Disable mod"
};

const std::string settingsFolder = "./VStancer/";
const std::string settingsFile = settingsFolder+"Settings.ini";
const std::string savedCarsFile = settingsFolder+"SavedCars.xml";
const std::string presetsFile = settingsFolder +"Presets.xml";

Vehicle vehicle;
Vehicle prevVehicle;
VehicleExtensions ext;
Player player;
Ped playerPed;

std::vector<Preset> presets;
std::vector<Preset> saved;

Controls controls;
Settings settings(settingsFile);

void ultraSlam(Vehicle handle, float camberFront, float camberRear, double distanceFront, double distanceRear) {
	auto numWheels = ext.GetNumWheels(handle);
	if (numWheels < 4)
		return;

	auto wheelPtr = ext.GetWheelsPtr(handle);  // pointer to wheel pointers

	auto offsetCamber = 0x008;
	auto offsetinvCamber = 0x010;
	auto offsetDistance = 0x02C;
	auto offsetHeight = 0x038;

	for (auto i = 0; i < numWheels; i++) {
		float camber;
		double distance;
		if (i == 0 || i ==  1) {
			camber = camberFront;
			distance = distanceFront;
		} else {
			camber = camberRear;
			distance = distanceRear;
		}

		float flip = i % 2 == 0 ? 1.0f : -1.0f; // cuz the wheels on the other side
		auto wheelAddr = *reinterpret_cast<uint64_t *>(wheelPtr + 0x008 * i);
		*reinterpret_cast<float *>(wheelAddr + offsetCamber) = camber * flip;
		*reinterpret_cast<float *>(wheelAddr + offsetinvCamber) = -camber * flip;
		*reinterpret_cast<double *>(wheelAddr + offsetDistance) = -distance * flip;
	}
}

void update_menu() {
	
}

void update_game() {
	if (!settings.EnableMod())
		return;

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
	}
	prevVehicle = vehicle;

	// do your slamming magic here, ikt
}

void init() {
	settings.ReadSettings(&controls);
	// Depending on how crappy the XML is this shit might crash and burn.
	presets = settings.ReadPresets(presetsFile);
	saved = settings.ReadPresets(savedCarsFile);
}

void main() {
	init();
	while (true) {
		update_menu();
		update_game();
		WAIT(0);
	}
}

void ScriptMain() {
	srand(GetTickCount());
	main();
}
