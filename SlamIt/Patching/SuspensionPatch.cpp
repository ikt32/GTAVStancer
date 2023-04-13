#include "SuspensionPatch.hpp"

#include "Hooking.hpp"

#include "../Memory/NativeMemory.hpp"
#include "../Util/Logger.hpp"

#include <inc/nativeCaller.h>

// Instructions that access suspension members, 1.0.1032.1
// GTA5.exe + F1023B - F3 0F11 43 28	- movss[rbx + 28], xmm0		; ???
// GTA5.exe + F10240 - F3 44 0F11 63 20 - movss[rbx + 20], xmm12	; ???
// GTA5.exe + F10246 - F3 44 0F11 4B 24 - movss[rbx + 24], xmm9		; ???
// GTA5.exe + F1024C - F3 0F11 73 30	- movss[rbx + 30], xmm6		; track width
// GTA5.exe + F10251 - F3 0F11 5B 34	- movss[rbx + 34], xmm3		; ???
// GTA5.exe + F10256 - F3 0F11 63 38	- movss[rbx + 38], xmm4		; height

// FiveM (1.0.505.2)
// FiveM... + F01E9B - F3 0F11 43 24	- movss[rbx + 28], xmm0		; ???
// FiveM... + F01EA0 - F3 0F11 5B 28	- movss[rbx + 20], xmm3		; ???
// FiveM... + F01EA5 - F3 44 0F11 63 20 - movss[rbx + 24], xmm12	; ???
// FiveM... + F01EAB - F3 0F11 4B 30	- movss[rbx + 30], xmm1		; track width
// FiveM... + F01EB0 - F3 0F11 5B 34	- movss[rbx + 34], xmm6		; ???
// FiveM... + F01EB5 - F3 0F11 63 38	- movss[rbx + 38], xmm4		; height
bool patched = false;

typedef void(*SetHeight_t)();

CallHookRaw<SetHeight_t>* g_SetHeight;

extern "C" void compare_height();
//extern "C" void original_thing();

void SetHeight_Stub() {
    compare_height();
    //original_thing();
}

const char* patt1604 = "\xF3\x0F\x11\x43\x28\xF3\x44\x0F\x11\x4B\x24\xF3\x44\x0F\x11\x63\x30\xF3\x44\x0F\x11\x73\x34\xF3\x0F\x11\x5B\x38";
const char* mask1604 = "xxx?x" "xxxx?x" "xxxx?x" "xxxx?x" "xxx?x";

void VStancer::PatchHeightReset() {
    if (getGameVersion() < 46) {
        LOG(ERROR, "[Patch] Incompatible game version, require >= b1604");
    }

    if (patched)
        return;

    LOG(INFO, "[Patch] Patching height reset");

    uintptr_t result = mem::FindPattern(patt1604, mask1604);
    uintptr_t offset = 23;

    if (result) {
        uintptr_t address = result + offset;

        LOG(INFO, "[Patch] Patching            @ 0x{:X}", address);

        g_SetHeight = HookManager::SetCallRaw<SetHeight_t>(address, SetHeight_Stub, 5);
        LOG(INFO, "[Patch] SetCall success     @ 0x{:X}", address);

        LOG(INFO, "[Patch] g_SetHeight address @ 0x{:X}", reinterpret_cast<char*>(g_SetHeight->fn));
        patched = true;
    }
    else {
        LOG(ERROR, "[Patch] No pattern found, aborting");
        patched = false;
    }
}

void VStancer::UnpatchHeightReset() {
    if (!patched)
        return;

    if (g_SetHeight) {
        delete g_SetHeight;
        g_SetHeight = nullptr;
        LOG(INFO, "[Patch] Unloaded");
        patched = false;
    }
}
