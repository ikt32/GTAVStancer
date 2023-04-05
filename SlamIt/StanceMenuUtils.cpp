#include "StanceMenuUtils.hpp"
#include "Util/UI.hpp"
#include <inc/natives.h>
#include <format>

std::string VStancer::GetKbEntryString(const std::string& existingString) {
    std::string val;
    UI::Notify("Enter value");
    MISC::DISPLAY_ONSCREEN_KEYBOARD(LOCALIZATION::GET_CURRENT_LANGUAGE() == 0, "FMMC_KEY_TIP8", "",
        existingString.c_str(), "", "", "", 64);
    while (MISC::UPDATE_ONSCREEN_KEYBOARD() == 0) {
        WAIT(0);
    }
    if (!MISC::GET_ONSCREEN_KEYBOARD_RESULT()) {
        UI::Notify("Cancelled value entry");
        return {};
    }

    std::string enteredVal = MISC::GET_ONSCREEN_KEYBOARD_RESULT();
    if (enteredVal.empty()) {
        UI::Notify("Cancelled value entry");
        return {};
    }

    return enteredVal;
}

bool VStancer::GetKbEntryInt(int& val) {
    UI::Notify("Enter value");
    MISC::DISPLAY_ONSCREEN_KEYBOARD(LOCALIZATION::GET_CURRENT_LANGUAGE() == 0, "FMMC_KEY_TIP8", "",
        std::format("{}", val).c_str(), "", "", "", 64);
    while (MISC::UPDATE_ONSCREEN_KEYBOARD() == 0) {
        WAIT(0);
    }
    if (!MISC::GET_ONSCREEN_KEYBOARD_RESULT()) {
        UI::Notify("Cancelled value entry");
        return false;
    }

    std::string intStr = MISC::GET_ONSCREEN_KEYBOARD_RESULT();
    if (intStr.empty()) {
        UI::Notify("Cancelled value entry");
        return false;
    }

    char* pEnd;
    int parsedValue = strtol(intStr.c_str(), &pEnd, 10);

    if (parsedValue == 0 && *pEnd != 0) {
        UI::Notify("Failed to parse entry.");
        return false;
    }

    val = parsedValue;
    return true;
}

bool VStancer::GetKbEntryFloat(float& val) {
    UI::Notify("Enter value");
    MISC::DISPLAY_ONSCREEN_KEYBOARD(LOCALIZATION::GET_CURRENT_LANGUAGE() == 0, "FMMC_KEY_TIP8", "",
        std::format("{:f}", val).c_str(), "", "", "", 64);
    while (MISC::UPDATE_ONSCREEN_KEYBOARD() == 0) {
        WAIT(0);
    }
    if (!MISC::GET_ONSCREEN_KEYBOARD_RESULT()) {
        UI::Notify("Cancelled value entry");
        return false;
    }

    std::string floatStr = MISC::GET_ONSCREEN_KEYBOARD_RESULT();
    if (floatStr.empty()) {
        UI::Notify("Cancelled value entry");
        return false;
    }

    char* pEnd;
    float parsedValue = strtof(floatStr.c_str(), &pEnd);

    if (parsedValue == 0.0f && *pEnd != 0) {
        UI::Notify("Failed to parse entry.");
        return false;
    }

    val = parsedValue;
    return true;
}
