#include "WindowsMessageDelegate.hpp"

std::shared_ptr<windows_message_delegate> windows_message_delegate::mInstance;

bool windows_message_delegate::onMessage(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam) {
    {
        std::scoped_lock m{mFuncsMutex};
        for (auto m_func : mFuncs) {
            m_func(wnd, message, wParam, lParam);
        }
    }
    return Mod::onMessage(wnd, message, wParam, lParam);
}

void windows_message_delegate::onFrame() {
    std::scoped_lock m{mFuncsMutex};
    for (auto m_func : mFrameFuncs) {
        m_func();
    }
}

size_t windows_message_delegate::add_msg_func(std::function<void(HWND, UINT message, WPARAM wParam, LPARAM lParam)> proc) {
    std::scoped_lock m{mFuncsMutex};

    mFuncs.push_back(proc);
    return mFuncs.size() - 1;
}

size_t windows_message_delegate::add_frame_func(std::function<void()> proc) {
    std::scoped_lock m{mFuncsMutex};

    mFrameFuncs.push_back(proc);
    return mFrameFuncs.size() - 1;
}

bool windows_message_delegate::remove_msg_func(size_t id) {
    std::scoped_lock m{mFuncsMutex};

    if (id >= mFuncs.size())
        return false;
    mFuncs.erase(std::next(mFuncs.begin(), id));
    return true;
}

bool windows_message_delegate::remove_frame_func(size_t id) {
    std::scoped_lock m{mFuncsMutex};

    if (id >= mFrameFuncs.size())
        return false;
    mFrameFuncs.erase(std::next(mFrameFuncs.begin(), id));
    return true;
}

extern "C" __declspec(dllexport) int onMessageDelegate(
    void (*proc)(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam)) {
    return windows_message_delegate::getInstance()->add_msg_func(proc);
}


extern "C" __declspec(dllexport) int onFrame(
    void (*proc)()) {
    return windows_message_delegate::getInstance()->add_frame_func(proc);
}
