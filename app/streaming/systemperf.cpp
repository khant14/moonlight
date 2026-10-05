#include "systemperf.h"

#include <QtGlobal>

#include "SDL_compat.h"

#ifdef Q_OS_WIN32
#include <windows.h>

namespace {

// These mirror PROCESS_POWER_THROTTLING_STATE and friends from newer Windows
// SDKs, so we can opt out regardless of the SDK we're built with.
struct MlPowerThrottlingState {
    ULONG Version;
    ULONG ControlMask;
    ULONG StateMask;
};
const int kProcessPowerThrottling = 4; // PROCESS_INFORMATION_CLASS::ProcessPowerThrottling
const ULONG kPowerThrottlingCurrentVersion = 1;
const ULONG kPowerThrottlingExecutionSpeed = 0x1;
const ULONG kPowerThrottlingIgnoreTimerResolution = 0x4;

void setProcessPowerThrottling(bool allowThrottling)
{
    typedef BOOL (WINAPI *SetProcessInformationFn)(HANDLE, int, LPVOID, DWORD);
    auto setProcessInformation = (SetProcessInformationFn)
            GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "SetProcessInformation");
    if (setProcessInformation == nullptr) {
        // Not available prior to Windows 8
        return;
    }

    MlPowerThrottlingState state = {};
    state.Version = kPowerThrottlingCurrentVersion;
    if (!allowThrottling) {
        // Opt out of EcoQoS (reduced clocks when Windows considers us a background
        // process, such as when the stream window isn't focused on battery power)
        // and keep honoring our 1 ms timer resolution request when occluded.
        state.ControlMask = kPowerThrottlingExecutionSpeed | kPowerThrottlingIgnoreTimerResolution;
        state.StateMask = 0;
    }
    // else: an empty ControlMask hands control back to the system default policy

    if (!setProcessInformation(GetCurrentProcess(), kProcessPowerThrottling, &state, sizeof(state))) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                    "SetProcessInformation(ProcessPowerThrottling) failed: %d",
                    (int)GetLastError());
    }
}

}

void SystemPerf::beginStreaming()
{
    setProcessPowerThrottling(false);
}

void SystemPerf::endStreaming()
{
    setProcessPowerThrottling(true);
}

#elif !defined(Q_OS_DARWIN)

// macOS is implemented in systemperf_mac.mm

void SystemPerf::beginStreaming()
{
}

void SystemPerf::endStreaming()
{
}

#endif
