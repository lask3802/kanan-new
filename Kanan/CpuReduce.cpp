#include "CpuReduce.hpp"
#include "Log.hpp"

#include <chrono>
#include <optional>
#include <format>
#include <TlHelp32.h>

// Lists of thread IDs and handles
// Do not add the watcher thread to these lists!
static std::set<DWORD> threadList = std::set<DWORD>();

// Watcher thread ID and handle
static DWORD tidWatcherThread = 0;
static HANDLE hWatcherThread = NULL;

// CPU mod settings
static int reductionPercentage = 0;
static bool onlyWhileMinimized = true;

// Window status
static bool isMinimized;
static std::optional<int> threadId;
static std::optional<HWND> mainHWND;
using namespace kanan;

const static UINT WM_THREAD_SLEEP = WM_APP + 987;
const static UINT IDT_TIMER = 0x3822;
static std::optional<int> sleepCounter;

static enum SleepStatus { RUNNING, STOP };

CpuReduce::CpuReduce() {
    
}

void SleepAndProcessMessage(int sleepMs, int runningMs) {
    Sleep(sleepMs);
    SetTimer(mainHWND.value(), IDT_TIMER, runningMs, [](auto hwnd, auto msg, auto lparam, auto dw) {
        kanan::log("WM_TIMER callback");
        KillTimer(hwnd, IDT_TIMER);
        PostMessage(mainHWND.value(), WM_THREAD_SLEEP, NULL, NULL);
    });
}

SleepStatus SleepAndProcessMsg(int totalSleepMs, int checkMinimizeStep) {
    Sleep(checkMinimizeStep);

    if (!sleepCounter) {
        sleepCounter =  checkMinimizeStep;
    } else {
        sleepCounter.value() += checkMinimizeStep;
    }
    //sleep until total seconds
    if (sleepCounter > totalSleepMs) {
        sleepCounter = std::nullopt;
        return STOP;
    }
    PostMessage(mainHWND.value(), WM_THREAD_SLEEP, NULL, NULL);
    return RUNNING;
}

bool CpuReduce::onMessage(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (!mainHWND) {
        mainHWND = wnd;
    }

    if (!threadId) {
        threadId = GetThreadId(GetCurrentThread());
        log(std::format("main thread id: {}", threadId.value()).c_str());
    }

    switch (message) {
    case WM_ACTIVATE:
        if (HIWORD(wParam) != 0) {
            isMinimized = true;
            PostMessage(wnd, WM_THREAD_SLEEP, NULL, NULL);
        } else {
            KillTimer(mainHWND.value(), IDT_TIMER);
            isMinimized = false;
        }
        break;

    case WM_THREAD_SLEEP:
        if (!isMinimized)
            break;
        kanan::log("WM_THREAD_SLEEP");
        SleepAndProcessMessage(300, 5);
        break;
    default: 
        break;
    }
    return true;
}

std::set<DWORD> CpuReduce::GetList(void) {
    // DO NOT MODIFY THREAD LIST HERE
    // This function is called by the ModifyWindow thread!
    // If we are in the middle of some code which loops through this
    // list, and we modify it here, we are FUUUUCKED :(
    return threadList;
}

bool CpuReduce::UpdateList(void) {
    threadList.clear();

    DWORD pid = GetCurrentProcessId();
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    THREADENTRY32 te;
    te.dwSize = sizeof(te);

    if (!Thread32First(hSnapshot, &te)) {
        CloseHandle(hSnapshot);
        return false;
    }

    do {
        if ((te.th32OwnerProcessID == pid) && (te.th32ThreadID != tidWatcherThread)) {
            threadList.insert(te.th32ThreadID);
        }
    } while (Thread32Next(hSnapshot, &te));
    CloseHandle(hSnapshot);
    return true;
}

bool CpuReduce::SpawnWatcher(void) {
    SECURITY_ATTRIBUTES ThreadAttributes;
    ThreadAttributes.bInheritHandle = false;
    ThreadAttributes.lpSecurityDescriptor = NULL;
    ThreadAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);

    DWORD tid;
    HANDLE hThr = CreateThread(&ThreadAttributes, 0, WatcherThreadMain, NULL, 0, &tid);
    if (hThr == NULL) {
        kanan::log("log: CreateThread for watcher thread failed.\n");
        return false;
    }

    hWatcherThread = hThr;
    tidWatcherThread = tid;

    if (!SetThreadPriority(hWatcherThread, THREAD_PRIORITY_TIME_CRITICAL)) {
        kanan::log("Warning: SetThreadPriority for watcher thread failed.\n");
    }
    return true;
}

void SleepAndCheckIsMinimized(DWORD totalSleepMs, DWORD checkMinimizeStep) {

    DWORD elapsed = 0u;

    for (auto timer = 0u; timer < totalSleepMs; timer += elapsed) {
        if (mainHWND) {
            MSG winMsg;
            while(PeekMessage(&winMsg, mainHWND.value(), WM_ACTIVATE, WM_ACTIVATE+1, PM_NOREMOVE)) {
                kanan::log(std::format("PeekMessage: {}", winMsg.message).c_str());
                if(winMsg.message == WM_ACTIVATE) {
                    if (HIWORD(winMsg.wParam) != 0)
                        isMinimized = true;
                    else
                        isMinimized = false;
                    kanan::log(std::format("PeekMessage isMinimized: {}", isMinimized).c_str());
                    if (!isMinimized) {
                        return;
                    }
                }
            }            

        } else {
            log("Main window handler not found!");
        }
        auto start = std::chrono::steady_clock::now();
        Sleep(checkMinimizeStep);
        elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
        kanan::log(std::format("step elapsed: {}ms", elapsed).c_str());
    }
}


DWORD WINAPI CpuReduce::WatcherThreadMain(LPVOID lpParam) {
    UpdateList(); // Get the initial list set up so ModifyWindow can find our HWnd
    while (true) {
        if (!isMinimized) {
            Sleep(1000);
        } else {
            Sleep(1000);
            /* UpdateList();
            SleepAll();
            SleepAndCheckIsMinimized(10000, 100);
            WakeAll();
            Sleep(50);*/
        }
    }
    return 0;
}


bool CpuReduce::SleepAll(void) {
    bool result = true;
    std::set<DWORD>::iterator it;
    if (!threadId)
        return false;
    for (it = threadList.begin(); it != threadList.end(); it++) {
        if (*it != threadId) {
            continue;            
        }
        HANDLE hThr = OpenThread(THREAD_SUSPEND_RESUME, false, *it);
        if (hThr == NULL) {
            log("log: Could not open handle to thread.  Id: 0x%lX\n", *it);
            result = false;
        } else {
            if (-1 == SuspendThread(hThr)) {
                log("log: Could not suspend thread.  Id: 0x%08lX\n", *it);
                result = false;
            }
            log("suspend thread Id: %i", *it);
            CloseHandle(hThr);
        }
    }
    return result;
}

bool CpuReduce::WakeAll(void) {
    bool result = true;
    std::set<DWORD>::iterator it;
    if (!threadId)
        return false;
    for (it = threadList.begin(); it != threadList.end(); it++) {
        if (*it != threadId) {
            continue;
        }
        HANDLE hThr = OpenThread(THREAD_SUSPEND_RESUME, false, *it);
        if (hThr == NULL) {
            log("log: Could not open handle to thread.  Id: 0x%lX\n", *it);
            result = false;
        } else {
            if (-1 == ResumeThread(hThr)) {
                log("log: Could not resume thread.  Id: 0x%08lX\n", *it);
                result = false;
            }
            CloseHandle(hThr);
        }
    }
    return result;
}

void CpuReduce::SetReductionPercentage(int percent) {
    reductionPercentage = percent;
}

int CpuReduce::GetReductionPercentage(void) {
    return reductionPercentage;
}

void CpuReduce::SetOptionOnlyMinimized(bool value) {
    onlyWhileMinimized = value;
}

bool CpuReduce::GetOptionOnlyMinimized(void) {
    return onlyWhileMinimized;
}