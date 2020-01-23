#define WIN32_LEAN_AND_MEAN
#define _USE_MATH_DEFINES

#include <windows.h>
#include "Utils/MemoryMgr.h"
#include "Utils/Trampoline.h"
#include "Utils/Patterns.h"

BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lparam);
HWND find_main_window();
std::tuple<int32_t, int32_t> GetWindowRes();

static float& fAspectRatio = Trampoline::MakeTrampoline(GetModuleHandle(nullptr))->Reference<float>();
static float& fFOVMult = Trampoline::MakeTrampoline(GetModuleHandle(nullptr))->Reference<float>();

bool KeyPressed(unsigned int keyCode) {
    return (GetKeyState(keyCode) & 0x8000) != 0;
}

__int64(__fastcall*hbsub_1BC31F0)(__int64);
__int64 __fastcall sub_1BC31F0(__int64 a) {
    uint32_t AspectRatioWidth, AspectRatioHeight;
    uint32_t ViewPortWidth, ViewPortHeight;
    float fCustomAspectRatioHor, fCustomAspectRatioVer;

    std::tie(ViewPortWidth, ViewPortHeight) = GetWindowRes();

    AspectRatioWidth = ViewPortWidth;
    AspectRatioHeight = ViewPortHeight;

    fCustomAspectRatioHor = static_cast<float>(AspectRatioWidth);
    fCustomAspectRatioVer = static_cast<float>(AspectRatioHeight);

    fAspectRatio = fCustomAspectRatioHor / fCustomAspectRatioVer;

    float f = std::round((2.0f * atan(((fAspectRatio) / (16.0f / 9.0f)) * tan((0.0087266462f * 10000.0f) / 2.0f * ((float)M_PI / 180.0f)))) * (180.0f / (float)M_PI) * 100.0f) / 100.0f;
    fFOVMult = f / 10000.0f;

    return hbsub_1BC31F0(a);
}

void FixAspectRatio() {
    std::unique_ptr<ScopedUnprotect::Unprotect> Protect = ScopedUnprotect::UnprotectSectionOrFullModule(GetModuleHandle(nullptr), ".text");

    using namespace std;
    using namespace Memory;
    using namespace hook;

    // Fiv FOV
    Trampoline* trampoline = Trampoline::MakeTrampoline(GetModuleHandle(nullptr));

    auto FOV_Pattern = get_pattern("48 83 C1 40 E8 ? ? 99 01", 4);

    // dword_2D1E83C
    auto setFieldOfView = get_pattern("F3 0F 59 05 ? ? ? ? E8 ? ? C2 00", 4);

    ReadCall(FOV_Pattern, hbsub_1BC31F0);
    InjectHook(FOV_Pattern, trampoline->Jump(sub_1BC31F0), PATCH_CALL);

    WriteOffsetValue(setFieldOfView, &fFOVMult);

    // Fix aspect ratio.
    void* setAspectRatio[] = {
        get_pattern("C7 80 58 02 00 00 3B 8E E3 3F", 6),
        get_pattern("C7 87 5C 03 00 00 3B 8E E3 3F", 6),
        get_pattern("C7 83 58 02 00 00 3B 8E E3 3F", 6),
    };

    for (;;) {
        for (void* pattern : setAspectRatio) {
            Patch<float>(pattern, fAspectRatio);
        }
    }
}

struct handle_data {
    unsigned long process_id;
    HWND window_handle;
};

BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lparam) {
    auto& data = *reinterpret_cast<handle_data*>(lparam);

    unsigned long process_id = 0;
    GetWindowThreadProcessId(handle, &process_id);

    if (data.process_id != process_id) {
        return TRUE;
    }

    data.window_handle = handle;
    return FALSE;
}

HWND find_main_window() {
    handle_data data{};

    data.process_id = GetCurrentProcessId();
    data.window_handle = nullptr;
    EnumWindows(enum_windows_callback, reinterpret_cast<LPARAM>(&data));

    return data.window_handle;
}

std::tuple<int32_t, int32_t> GetWindowRes() {
    WINDOWINFO info = {};
    info.cbSize = sizeof(WINDOWINFO);
    GetWindowInfo(find_main_window(), &info);
    int32_t w = info.rcClient.right - info.rcClient.left;
    int32_t h = info.rcClient.bottom - info.rcClient.top;
    return std::make_tuple(w, h);
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
