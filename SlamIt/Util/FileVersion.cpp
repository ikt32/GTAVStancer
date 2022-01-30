#include "FileVersion.hpp"

bool operator==(const SVersion& a, const SVersion& b) {
    return a.Minor == b.Minor && a.Build == b.Build;
}

bool operator<=(const SVersion& a, const SVersion& b) {
    if (a.Minor <= b.Minor && a.Build <= b.Build)
        return true;
    if (a.Minor == b.Minor)
        if (a.Build <= b.Build)
            return true;
    return false;
}
