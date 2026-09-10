#include "ft_pages.h"

#include <lvgl.h>
#include <src/font/lv_symbol_def.h>

#include <cstdint>

void FactoryPages::Initialize() {
    if (initialized_) return;  // 幂等

    main_scr_ = lv_screen_active();  // 复用当前激活屏幕作为主屏（保留鼠标 cursor）

    // 卡片容器：可滚动、垂直排列的 flex 列
    card_cont_ = lv_obj_create(main_scr_);
    lv_obj_set_size(card_cont_, LV_PCT(100), LV_PCT(100));  // 占满全屏
    lv_obj_set_flex_flow(card_cont_, LV_FLEX_FLOW_COLUMN);  // 垂直排列
    lv_obj_set_scroll_dir(card_cont_, LV_DIR_VER);          // 垂直滚动
    lv_obj_set_style_pad_row(card_cont_, 8, 0);             // 卡片间距 8px

    initialized_ = true;
}

void FactoryPages::Deinitialize() {
    if (!initialized_) return;

    if (card_cont_) lv_obj_delete(card_cont_);  // 删除容器（连带所有卡片）
    card_cont_ = nullptr;
    main_scr_  = nullptr;
    items_.clear();
    initialized_ = false;
}

void FactoryPages::AddTest(
    const char* title, std::function<void()> on_start, std::function<void()> on_stop
) {
    if (!initialized_) Initialize();  // 懒初始化

    size_t idx = items_.size();
    items_.push_back({title, std::move(on_start), std::move(on_stop)});

    // 卡片 = 一个满宽按钮
    lv_obj_t* card = lv_button_create(card_cont_);
    lv_obj_set_width(card, LV_PCT(100));  // 满宽
    lv_obj_set_height(card, 48);          // 卡片高度

    lv_obj_t* label = lv_label_create(card);
    lv_label_set_text(label, items_[idx].title.c_str());
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_event_cb(card, onCardClicked, LV_EVENT_CLICKED, (void*)(intptr_t)idx);
}

void FactoryPages::onCardClicked(lv_event_t* e) {
    FactoryPages& self = GetInstance();
    size_t        idx  = (size_t)(intptr_t)lv_event_get_user_data(e);
    if (idx >= self.items_.size()) return;

    const TestItem& item = self.items_[idx];

    // 进入详情页前保存主界面滚动位置（返回时恢复）
    self.saved_scroll_y_ = lv_obj_get_scroll_y(self.card_cont_);

    // 详情页 = 新建一个 screen
    lv_obj_t* scr = lv_obj_create(NULL);

    // 标题
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, item.title.c_str());
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    // 「开始」按钮
    lv_obj_t* btn_start = lv_button_create(scr);
    lv_obj_align(btn_start, LV_ALIGN_CENTER, 0, -20);
    lv_obj_t* lbl_start = lv_label_create(btn_start);
    lv_label_set_text(lbl_start, "Start");
    lv_obj_center(lbl_start);
    lv_obj_add_event_cb(btn_start, onStart, LV_EVENT_CLICKED, (void*)(intptr_t)idx);

    // 「停止」按钮
    lv_obj_t* btn_stop = lv_button_create(scr);
    lv_obj_align(btn_stop, LV_ALIGN_CENTER, 0, 40);
    lv_obj_t* lbl_stop = lv_label_create(btn_stop);
    lv_label_set_text(lbl_stop, "Stop");
    lv_obj_center(lbl_stop);
    lv_obj_add_event_cb(btn_stop, onStop, LV_EVENT_CLICKED, (void*)(intptr_t)idx);

    // 「返回」按钮
    lv_obj_t* btn_back = lv_button_create(scr);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_t* lbl_back = lv_label_create(btn_back);
    lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
    lv_obj_center(lbl_back);
    lv_obj_add_event_cb(btn_back, onBack, LV_EVENT_CLICKED, scr);

    lv_screen_load(scr);  // 切到详情页
}

void FactoryPages::onStart(lv_event_t* e) {
    FactoryPages& self = GetInstance();
    size_t        idx  = (size_t)(intptr_t)lv_event_get_user_data(e);
    if (idx < self.items_.size() && self.items_[idx].on_start) self.items_[idx].on_start();
}

void FactoryPages::onStop(lv_event_t* e) {
    FactoryPages& self = GetInstance();
    size_t        idx  = (size_t)(intptr_t)lv_event_get_user_data(e);
    if (idx < self.items_.size() && self.items_[idx].on_stop) self.items_[idx].on_stop();
}

void FactoryPages::onBack(lv_event_t* e) {
    FactoryPages& self   = GetInstance();
    lv_obj_t*     detail = (lv_obj_t*)lv_event_get_user_data(e);

    lv_screen_load(self.main_scr_);     // 先切回主屏
    if (detail) lv_obj_delete(detail);  // 再删除详情页（反序会删到 active 屏）

    // 恢复主界面滚动位置（LVGL 切屏后不保留 scroll，需手动恢复）
    lv_obj_scroll_to_y(self.card_cont_, self.saved_scroll_y_, LV_ANIM_OFF);
}
