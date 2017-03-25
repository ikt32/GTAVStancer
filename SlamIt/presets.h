#pragma once
#include <string>
class Settings;

class Preset
{
public:
	struct WheelInfo {
		float Camber;
		double Distance;
		float height;
	}; 
	
	Preset(WheelInfo front, WheelInfo rear, const std::string &name, const std::string &plate);
	~Preset();

	/*
	 * If this is NOT a preset then it's a saved cahr.
	 */
	bool IsPreset();

	WheelInfo Front;
	WheelInfo Rear;

	std::string Plate();
	std::string Name();

private:
	const std::string reservedPlate = "_GENERICPRESET_";
	std::string name;
	std::string plate;
};

