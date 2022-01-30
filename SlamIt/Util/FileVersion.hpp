#pragma once

struct SVersion {
    int Minor;
    int Build;
};

bool operator==(const SVersion& a, const SVersion& b);
bool operator<=(const SVersion& a, const SVersion& b);
