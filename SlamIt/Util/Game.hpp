#pragma once
#include <inc/natives.h>

namespace Util {
    inline bool VehicleAvailable(Vehicle vehicle, Ped playerPed, bool checkSeat) {
        bool seatOk = true;
        if (checkSeat)
            seatOk = playerPed == VEHICLE::GET_PED_IN_VEHICLE_SEAT(vehicle, -1, 0);

        return vehicle != 0 &&
            ENTITY::DOES_ENTITY_EXIST(vehicle) &&
            PED::IS_PED_IN_VEHICLE(playerPed, vehicle, false) &&
            seatOk;
    }
}
