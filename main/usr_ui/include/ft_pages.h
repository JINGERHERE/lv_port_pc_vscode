#pragma once

#include <lvgl.h>

class FactoryPages {
public:
    static FactoryPages& GetInstance() {
        static FactoryPages instance;
        return instance;
    }

    FactoryPages(const FactoryPages&)            = delete;
    FactoryPages& operator=(const FactoryPages&) = delete;

    FactoryPages()  = default;
    ~FactoryPages() = default;

public:
    void Initialize();

    void Deinitialize();

private:
    bool initialized_ = false;

    // lv_obj_t* scr_ = lv_scr_act();
    lv_obj_t* scr_act_ = nullptr;
};