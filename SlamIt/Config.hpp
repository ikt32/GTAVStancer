#pragma once
#include <inc/types.h>
#include <optional>
#include <string>
#include <vector>

class CConfig {
public:
    enum class ESaveType {
        Specific,       // [ID] writes Model + Plate
        GenericModel,   // [ID] writes Model
        GenericNone,    // [ID] writes none
    };

    struct SSuspensionParams {
        float Camber;
        float TrackWidth;
        float Height;
    };

    struct SSuspensionTuning {
        SSuspensionParams Front;
        SSuspensionParams Rear;
        float VisualHeight;
    };

    struct SWheelColliderParams {
        float TyreRadius;
        float TyreWidth;
        float RimRadius;
    };

    struct SWheelRenderParams {
        float WheelSize;
        float WheelWidth;
        int WheelType;
        int ModIndex;
    };

    struct SWheelTuning {
        SWheelColliderParams Front;
        SWheelColliderParams Rear;
        SWheelRenderParams Visual;
    };

    struct SModAdjustment {
        int ModSlot;
        int ModIndex;

        SSuspensionParams FrontAdjust;
        SSuspensionParams RearAdjust;
    };
    
    CConfig() = default;
    static CConfig Read(const std::string& configFile);

    void Write();
    bool Write(const std::string& newName, Hash model, std::string plate, ESaveType saveType);

    std::string Name;

    Hash ModelHash = 0;
    std::string ModelName;
    std::string Plate;

    SSuspensionTuning Suspension = {};
    SWheelTuning Wheels = {};

    struct {
        // -1: Ignore.
        // 0: Restore to 1000
        // 1: Restore to x
        // 2: Restore to x
        int DamageLoweringLevel = -1;
    } Misc;

    std::vector<SModAdjustment> ModAdjustments;
};

CConfig::SSuspensionParams operator+(const CConfig::SSuspensionParams& a, const CConfig::SSuspensionParams& b);
