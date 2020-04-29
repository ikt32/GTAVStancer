#include "presets.h"

const std::string Preset::reservedPlate = "_GENERICPRESET_";

Preset::Preset(Suspension front, 
               Suspension rear, 
               WheelPhys frontWheels,
               WheelPhys rearWheels,
               WheelVis visualSize,
               float visualHeight,
               const std::string &name, 
               const std::string &plate)  : FrontSuspension(front), 
                                      RearSuspension(rear), 
                                      FrontWheels(frontWheels), 
                                      RearWheels(rearWheels),
                                      VisualSize(visualSize),
                                      VisualHeight(visualHeight),
                                      name(name), 
                                      plate(plate) { }

Preset::~Preset() {
}

std::string Preset::ReservedPlate() {
    return reservedPlate;
}

bool Preset::IsPreset() {
    return plate == reservedPlate;
}

std::string Preset::Plate() {
    return plate;
}
std::string Preset::Name() {
    return name;
}

bool Preset::operator==(const Preset &other) const {
    if (name == other.name &&
        plate == other.plate) {
        return true;
    }
    return false;
}
