#include "AddonSpawnerCache.hpp"
#include "Paths.hpp"

#include <fstream>

namespace {
    std::unordered_map<Hash, std::string> hashCache;
}

const std::unordered_map<Hash, std::string>& ASCache::Get() {
    if (!hashCache.empty())
        return hashCache;

    auto cacheFile = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) / "AddonSpawner/hashes.cache";
    std::ifstream infile(cacheFile);
    if (infile.is_open()) {
        Hash hash;
        std::string name;
        while (infile >> hash >> name) {
            hashCache.insert({ hash, name });
        }
    }
    return hashCache;
}
