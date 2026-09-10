#include "ft_pages.h"

#include <lvgl.h>
#include <src/font/lv_symbol_def.h>

#include <cstdint>

void FactoryPages::Initialize() {
    if (initialized_) return;  // 幂等

    main_scr_ = lv_screen_active();  // 复用当前激活屏幕作为主屏（保留鼠标 cursor）
    lv_obj_set_style_bg_color(main_scr_, lv_color_white(), 0);  // 主屏纯白（默认是浅灰）

    main_group_ = lv_group_get_default();  // 复用默认焦点组（卡片创建时自动加入）

    // ===== ===== ===== ===== =====
    // 焦点样式：橙色底色 + 斜投影
    // ===== ===== ===== ===== =====
    lv_style_init(&focus_style_);
    lv_style_set_bg_color(&focus_style_, lv_color_hex(0xFF9800));
    lv_style_set_bg_opa(&focus_style_, LV_OPA_COVER);
    lv_style_set_shadow_width(&focus_style_, 12);
    lv_style_set_shadow_opa(&focus_style_, LV_OPA_60);
    lv_style_set_shadow_offset_x(&focus_style_, 8);
    lv_style_set_shadow_offset_y(&focus_style_, 8);

    // ===== ===== ===== ===== =====
    // 卡片容器：可滚动、垂直排列的 flex 列
    // ===== ===== ===== ===== =====
    card_cont_ = lv_obj_create(main_scr_);
    lv_obj_set_size(card_cont_, LV_PCT(90), LV_PCT(90));  // 容器大小
    lv_obj_align(card_cont_, LV_ALIGN_CENTER, 0, 0);      // 居中

    lv_obj_set_flex_flow(card_cont_, LV_FLEX_FLOW_COLUMN);  // 垂直排列
    lv_obj_set_scroll_dir(card_cont_, LV_DIR_VER);          // 垂直滚动
    lv_obj_set_style_pad_row(card_cont_, 16, 0);            // 内部间距
    lv_obj_set_flex_align(
        card_cont_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER
    );  // 交叉轴水平居中

    lv_obj_set_style_bg_color(card_cont_, lv_color_white(), 0);  // 容器背景
    lv_obj_set_style_border_width(card_cont_, 1, 0);             // 容器边框

    initialized_ = true;
}

void FactoryPages::Deinitialize() {
    if (!initialized_) return;

    if (card_cont_) lv_obj_delete(card_cont_);  // 删除容器（连带所有卡片）
    card_cont_ = nullptr;
    main_scr_  = nullptr;

    if (detail_group_) lv_group_delete(detail_group_);  // 防御：清理残留的详情页焦点组
    detail_group_ = nullptr;
    main_group_   = nullptr;

    lv_style_reset(&focus_style_);  // 释放焦点样式

    items_.clear();
    initialized_ = false;
}

void FactoryPages::AddTest(
    const char*           title,
    std::function<void()> on_start,
    std::function<void()> on_stop,
    bool                  keep_on_back
) {
    if (!initialized_) Initialize();  // 懒初始化

    size_t idx = items_.size();
    items_.push_back({title, std::move(on_start), std::move(on_stop), keep_on_back});

    // 卡片 = 一个按钮（水平居中由 card_cont_ 的 flex 交叉轴对齐负责）
    lv_obj_t* card = lv_button_create(card_cont_);
    lv_obj_set_width(card, LV_PCT(70));  // 卡片宽度
    lv_obj_set_height(card, 48);         // 卡片高度

    lv_obj_t* label = lv_label_create(card);
    lv_label_set_text(label, items_[idx].title.c_str());
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_event_cb(card, onCardClicked, LV_EVENT_CLICKED, (void*)(intptr_t)idx);
    lv_obj_add_style(card, &focus_style_, LV_STATE_FOCUS_KEY);     // 焦点高亮（橙底 + 阴影）
    lv_obj_set_style_width(card, LV_PCT(80), LV_STATE_FOCUS_KEY);  // 焦点时变宽到 80%
    // lv_obj_set_style_bg_color(card, lv_color_white(), LV_STATE_FOCUS_KEY);  // 焦点时背景为白
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
    // lv_obj_set_size(scr, LV_PCT(70), LV_PCT(70));  // 容器大小
    // lv_obj_align(scr, LV_ALIGN_CENTER, 0, 0);      // 居中

    // 详情页独立焦点组：让 Start/Stop/Back 只在本页内循环，不受主界面卡片干扰
    self.detail_group_ = lv_group_create();
    lv_group_set_default(self.detail_group_);  // 下面创建的按钮自动加入它

    // 标题
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, item.title.c_str());
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    // 「开始」按钮
    lv_obj_t* btn_start = lv_button_create(scr);
    lv_obj_align(btn_start, LV_ALIGN_CENTER, -40, 0);
    lv_obj_t* lbl_start = lv_label_create(btn_start);
    lv_label_set_text(lbl_start, "Start");
    lv_obj_center(lbl_start);
    lv_obj_add_event_cb(btn_start, onStart, LV_EVENT_CLICKED, (void*)(intptr_t)idx);
    lv_obj_add_style(btn_start, &self.focus_style_, LV_STATE_FOCUS_KEY);  // 焦点高亮

    // 「停止」按钮
    lv_obj_t* btn_stop = lv_button_create(scr);
    lv_obj_align(btn_stop, LV_ALIGN_CENTER, 40, 0);
    lv_obj_t* lbl_stop = lv_label_create(btn_stop);
    lv_label_set_text(lbl_stop, "Stop");
    lv_obj_center(lbl_stop);
    lv_obj_add_event_cb(btn_stop, onStop, LV_EVENT_CLICKED, (void*)(intptr_t)idx);
    lv_obj_add_style(btn_stop, &self.focus_style_, LV_STATE_FOCUS_KEY);  // 焦点高亮

    // 「返回」按钮
    lv_obj_t* btn_back = lv_button_create(scr);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_t* lbl_back = lv_label_create(btn_back);
    lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
    lv_obj_center(lbl_back);
    lv_obj_add_event_cb(btn_back, onBack, LV_EVENT_CLICKED, (void*)(intptr_t)idx);
    lv_obj_add_style(btn_back, &self.focus_style_, LV_STATE_FOCUS_KEY);  // 焦点高亮

    lv_group_set_default(self.main_group_);  // 恢复默认组，避免后续 AddTest 的卡片误入
    setAllIndevGroup(self.detail_group_);    // 让按键/旋钮导航详情页

    lv_screen_load(scr);  // 切到详情页
}

void FactoryPages::onStart(lv_event_t* e) {
    FactoryPages& self = GetInstance();
    size_t        idx  = (size_t)(intptr_t)lv_event_get_user_data(e);

    // 索引超出范围
    if (idx >= self.items_.size()) return;

    // 不重复运行
    if (self.items_[idx].is_running) return;

    if (self.items_[idx].on_start) {
        self.items_[idx].on_start();
        self.items_[idx].is_running = true;
    }
}

void FactoryPages::onStop(lv_event_t* e) {
    FactoryPages& self = GetInstance();
    size_t        idx  = (size_t)(intptr_t)lv_event_get_user_data(e);
    if (idx >= self.items_.size()) return;

    // 停止
    if (self.items_[idx].on_stop) {
        self.items_[idx].on_stop();
    }

    // 无论 on_stop 是否为空，状态都复位
    self.items_[idx].is_running = false;
}

void FactoryPages::onBack(lv_event_t* e) {
    FactoryPages& self = GetInstance();
    size_t        idx  = (size_t)(intptr_t)lv_event_get_user_data(e);

    // 返回时：若测试仍在运行且未配置「返回保留」，先补一次 stop，避免功能残留/冲突
    if (idx < self.items_.size() && self.items_[idx].is_running && !self.items_[idx].keep_on_back) {
        // 停止
        if (self.items_[idx].on_stop) {
            self.items_[idx].on_stop();
        }
        // 更新状态
        self.items_[idx].is_running = false;
    }

    lv_obj_t* detail = lv_screen_active();  // 详情页（此刻仍是 active 屏）

    lv_screen_load(self.main_scr_);     // 先切回主屏
    if (detail) lv_obj_delete(detail);  // 再删除详情页（反序会删到 active 屏）

    // 焦点组切回主界面，并释放详情页焦点组
    setAllIndevGroup(self.main_group_);
    if (self.detail_group_) lv_group_delete(self.detail_group_);
    self.detail_group_ = nullptr;

    // 恢复主界面滚动位置（LVGL 切屏后不保留 scroll，需手动恢复）
    lv_obj_scroll_to_y(self.card_cont_, self.saved_scroll_y_, LV_ANIM_OFF);
}

void FactoryPages::setAllIndevGroup(lv_group_t* g) {
    lv_indev_t* indev = NULL;
    while ((indev = lv_indev_get_next(indev))) {
        lv_indev_set_group(indev, g);
    }
}
