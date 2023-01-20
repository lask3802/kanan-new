#include "Mod.hpp"
#include <Windows.h>
#include <set>

namespace kanan {

class CpuReduce : public kanan::Mod {

    public:
    CpuReduce();
    // Kanan event
    bool onMessage(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam) override;
    

    private:

    // Thread list (for CModifyWindow)
    static std::set<DWORD> GetList(void);
    // CPU mod functions
    static bool SpawnWatcher(void);
    static void SetReductionPercentage(int percent);
    static int GetReductionPercentage(void);
    static void SetOptionOnlyMinimized(bool onlyMinimized);
    static bool GetOptionOnlyMinimized(void);

private:
    static DWORD WINAPI WatcherThreadMain(LPVOID lpParam);
    static bool UpdateList(void);
    static bool SleepAll(void);
    static bool WakeAll(void);
};
} // namespace kanan