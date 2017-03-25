#include "presets.h"

Preset::Preset(WheelInfo front, WheelInfo rear, const std::string &name, const std::string &plate) 
	: Front(front)
	, Rear(rear)
	, name(name)
	, plate(plate) {
	// lmao nothing
}

Preset::~Preset() {
}

bool Preset::IsPreset() {
	return plate == reservedPlate;
}

std::string Preset::Plate() {}
std::string Preset::Name() {}
