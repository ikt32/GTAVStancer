#pragma once
#include <inc/types.h>
#include <string>
#include <unordered_map>

namespace ASCache {
    const std::unordered_map<Hash, std::string>& Get();
}