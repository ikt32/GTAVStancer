#pragma once
#include <string>
class Settings;

class Preset
{
public:
    struct Suspension {
        float Camber;
        float TrackWidth;
        float Height;
    }; 

    struct WheelPhys {
        float TyreRadius;
        float TyreWidth;
        float RimRadius;
    };

    struct WheelVis {
        float WheelSize;
        float WheelWidth;
        int WheelType;
        int WheelIndex;
    };
    
    Preset(Suspension frontSuspension, 
           Suspension rearSuspension, 
           WheelPhys frontWheels,
           WheelPhys rearWheels,
           WheelVis visualSize,
           float visualHeight,
           const std::string &name, 
           const std::string &plate);
    ~Preset();
    static std::string ReservedPlate();
    /*
     * If this is NOT a preset then it's a saved car.
     */
    bool IsPreset();

    Suspension FrontSuspension;
    Suspension RearSuspension;
    WheelPhys FrontWheels;
    WheelPhys RearWheels;
    WheelVis VisualSize;
    float VisualHeight;

    std::string Plate();
    std::string Name();

    bool operator==(const Preset &other) const;

private:
    const static std::string reservedPlate;
    std::string name;
    std::string plate;
};

