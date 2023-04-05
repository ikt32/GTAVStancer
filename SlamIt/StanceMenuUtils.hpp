#pragma once

#include <string>

namespace VStancer {
    std::string GetKbEntryString(const std::string& existingString);
    bool GetKbEntryInt(int& val);
    bool GetKbEntryFloat(float& val);
}
