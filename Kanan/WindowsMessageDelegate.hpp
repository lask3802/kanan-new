#pragma once
#include "Mod.hpp"

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

class windows_message_delegate : public kanan::Mod {
private:
    static std::shared_ptr<windows_message_delegate> mInstance;

    std::vector<std::function<void (HWND, UINT message, WPARAM wParam, LPARAM lParam)>> mFuncs;
    std::vector<std::function<void ()>> mFrameFuncs;
    std::mutex mFuncsMutex;

public:
    bool onMessage(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam) override;
    void onFrame() override;
    //return identifier for functions
    size_t add_msg_func(std::function<void(HWND, UINT message, WPARAM wParam, LPARAM lParam)>);
    size_t add_frame_func(std::function<void()>);
    //remove function with id
    bool remove_msg_func(size_t id);
    bool remove_frame_func(size_t id);

    static std::shared_ptr<windows_message_delegate> getInstance() {
        std::scoped_lock m;
        if (!mInstance) {
            mInstance = std::make_shared<windows_message_delegate>();
        }
        return mInstance;
    }
};

