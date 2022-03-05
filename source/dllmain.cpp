#define DEFAULT_ASPECT_RATIO 16.0f / 9.0f
#define DEFAULT_FOV_MULT 0.0087266462f

#include "Utils/MemoryMgr.h"
#include "Utils/Trampoline.h"
#include "Utils/Patterns.h"
#include "Avail/Screen.h"

using namespace std;
using namespace hook;
using namespace Memory::VP;
using namespace avail;

intptr_t(__fastcall *hbUpdateRoutine)(intptr_t);
intptr_t __fastcall UpdateRoutine(intptr_t a) {
    SetAspectRatio();
    SetFOVMult(DEFAULT_FOV_MULT, DEFAULT_ASPECT_RATIO);

    return hbUpdateRoutine(a);
}

void FixAspectRatio() {
    Trampoline* trampoline = Trampoline::MakeTrampoline(GetModuleHandle(nullptr));

    auto updateRoutine = get_pattern("E8 ? ? ? ? 33 C0 48 8B CF 48 89 87 ? ? ? ?");
    ReadCall(updateRoutine, hbUpdateRoutine);
    InjectHook(updateRoutine, trampoline->Jump(UpdateRoutine), PATCH_CALL);

    // Fiv FOV
    auto setFieldOfView = get_pattern("F3 0F 59 05 ? ? ? ? E8 ? ? ? ? F3 0F 10 2D ? ? ? ?", 4);
    WriteOffsetValue(setFieldOfView, &fFOVMult);

    // Fix aspect ratio.
    void* setAspectRatio[] = {
        get_pattern("C7 80 58 02 00 00 3B 8E E3 3F", 6),
        get_pattern("C7 87 5C 03 00 00 3B 8E E3 3F", 6),
        get_pattern("C7 83 58 02 00 00 3B 8E E3 3F", 6),
    };

    for (;;) {
        for (void* pattern : setAspectRatio)
            Patch<float>(pattern, fAspectRatio);
    }
}

void Init() {
    FixAspectRatio();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Init, 0, 0, 0);
    }
    return TRUE;
}

