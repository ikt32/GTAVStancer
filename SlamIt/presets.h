#pragma once
#include <string>
class Settings;

class Preset
{
public:
	struct WheelInfo {
		float Camber;
		float Distance;
		float Height;
	}; 
	
	Preset(WheelInfo front, WheelInfo rear, const std::string &name, const std::string &plate);
	~Preset();
	static std::string ReservedPlate();
	/*
	 * If this is NOT a preset then it's a saved cahr.
	 */
	bool IsPreset();

	WheelInfo Front;
	WheelInfo Rear;

	std::string Plate();
	std::string Name();

	bool Preset::operator==(const Preset &other) const;

private:
	const static std::string reservedPlate;
	std::string name;
	std::string plate;
};

