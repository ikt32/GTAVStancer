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

extern "C" void compare_height();

void SetHeight_Stub() {
    compare_height();
}

namespace {
    typedef void(*SetHeight_t)();

    std::unique_ptr<CallHookRaw<SetHeight_t>> SetHeightHook;

    const char* Patt1604 = "\xF3\x0F\x11\x43\x28" "\xF3\x44\x0F\x11\x4B\x24"
                           "\xF3\x44\x0F\x11\x63\x30" "\xF3\x44\x0F\x11\x73\x34" "\xF3\x0F\x11\x5B\x38";
    const char* Mask1604 = "xxx?x" "xxxx?x"
                           "xxxx?x" "xxxx?x" "xxx?x";

    uintptr_t SuspensionPatchAddr = 0;
    bool Patched = false;
    bool PatchFailed = false;
    bool LoggedPatch = false;
    bool LoggedUnpatch = false;
}

// Always expect this to be called
void VStancer::PatchHeightReset() {
    if (getGameVersion() < 46 && !LoggedPatch) {
        LOG(ERROR, "[Patch] Incompatible game version, require >= b1604");
    }

    if (Patched)
        return;

    if (SuspensionPatchAddr == 0 && !PatchFailed) {
        LOG(INFO, "[Patch] Patching Height Reset");
        auto address = mem::FindPattern(Patt1604, Mask1604);
        if (address) {
            SuspensionPatchAddr = address + 23;
            LOG(INFO, "[Patch] Height Reset found    @ 0x{:X}", SuspensionPatchAddr);
        }
        else {
            LOG(ERROR, "[Patch] Height Reset not found");
            PatchFailed = true;
        }
    }

    if (SuspensionPatchAddr != 0) {
        SetHeightHook = std::unique_ptr<CallHookRaw<SetHeight_t>>(HookManager::SetCallRaw<SetHeight_t>(
            SuspensionPatchAddr,
            SetHeight_Stub,
            5
        ));

        if (!LoggedPatch) {
            LOG(INFO, "[Patch] SetCall success       @ 0x{:X}", SuspensionPatchAddr);
            LOG(INFO, "[Patch] SetHeightHook address @ 0x{:X}", reinterpret_cast<uintptr_t>(SetHeightHook->fn));
        }
        Patched = true;
    }
    LoggedPatch = true;
}

void VStancer::UnpatchHeightReset() {
    if (!Patched)
        return;

    if (SetHeightHook) {
        SetHeightHook.reset();
        if (!LoggedUnpatch) {
            LOG(INFO, "[Patch] Unloaded");
            LoggedUnpatch = true;
        }
        Patched = false;
    }
}

bool VStancer::GetPatchStatus() {
    return Patched;
}

bool VStancer::GetPatchFailed() {
    return PatchFailed;
}
