#include "ft_pages.h"

#include <lvgl.h>
#include <src/font/lv_symbol_def.h>

#include <cstdint>

namespace {
    // ===== 主界面扁平风格参数（集中定义，便于调整） =====
    constexpr int32_t  CARD_H          = 48;        // 卡片高度
    constexpr int32_t  ZONE_H          = 56;        // 选择区域色块高度（卡片上下各留 4px）
    constexpr uint32_t ZONE_COLOR      = 0x569CD6;  // 选择区域色块颜色（蓝）
    constexpr uint32_t ENTER_SYM_COLOR = 0x333333;  // 进入按钮图标的常态颜色
    constexpr int32_t  ENTER_BTN_WH    = 36;        // 进入按钮边长
}  // namespace

void FactoryPages::Initialize() {
    if (initialized_) return;  // 幂等

    main_scr_ = lv_screen_active();  // 复用当前激活屏幕作为主屏（保留鼠标 cursor）
    lv_obj_set_style_bg_color(main_scr_, lv_color_white(), 0);  // 主屏纯白（默认是浅灰）

    main_group_ = lv_group_get_default();  // 复用默认焦点组
    if (!main_group_) {                    // 宿主未创建默认组时自建（ESP32 端兜底）
        main_group_ = lv_group_create();
        lv_group_set_default(main_group_);
    }
    lv_group_set_wrap(main_group_, false);  // 焦点不循环：滚到首/尾即停（Bug2 修复）

    // ===== ===== ===== ===== =====
    // 选择区域色块：先创建 → z 序最低；卡片全透明，色块从卡片底下透出
    // ===== ===== ===== ===== =====
    select_zone_ = lv_obj_create(main_scr_);
    lv_obj_remove_style_all(select_zone_);              // 裸对象：无主题样式干扰
    lv_obj_set_size(select_zone_, LV_PCT(72), ZONE_H);  // 宽 72% 屏（比卡片略宽形成条带感）
    lv_obj_align(select_zone_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(select_zone_, lv_color_hex(ZONE_COLOR), 0);
    lv_obj_set_style_bg_opa(select_zone_, LV_OPA_COVER, 0);  // 不透明
    lv_obj_clear_flag(select_zone_, LV_OBJ_FLAG_CLICKABLE);  // 不拦截点击

    // ===== ===== ===== ===== =====
    // 卡片容器：可滚动、垂直排列的 flex 列（扁平：全透明、无边框、无滚动条）
    // ===== ===== ===== ===== =====
    card_cont_ = lv_obj_create(main_scr_);
    lv_obj_set_size(card_cont_, LV_PCT(90), LV_PCT(90));  // 容器大小
    lv_obj_align(card_cont_, LV_ALIGN_CENTER, 0, 0);      // 居中

    lv_obj_set_flex_flow(card_cont_, LV_FLEX_FLOW_COLUMN);        // 垂直排列
    lv_obj_set_scroll_dir(card_cont_, LV_DIR_VER);                // 垂直滚动
    lv_obj_set_scroll_snap_y(card_cont_, LV_SCROLL_SNAP_CENTER);  // 垂直滚动时，保持卡片居中
    lv_obj_set_scrollbar_mode(card_cont_, LV_SCROLLBAR_MODE_ON);  // 滚动条
    lv_obj_set_flex_align(
        card_cont_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER
    );  // 交叉轴水平居中

    lv_obj_set_style_bg_opa(card_cont_, LV_OPA_TRANSP, 0);  // 容器透明（露出主屏白底）
    lv_obj_set_style_border_width(card_cont_, 2, 0);        // 去边框（扁平）
    lv_obj_set_style_pad_row(card_cont_, 16, 0);            // 卡片间距

    // 上下对称 padding：让第一张/最后一张卡片也能滚到正中央与色块重合
    // （也是未来圆屏「两侧卡片缩小」方案的地基）
    lv_obj_update_layout(card_cont_);
    int32_t pad = (lv_obj_get_height(card_cont_) - CARD_H) / 2;  // 首尾卡片和父级容器边缘的间距
    lv_obj_set_style_pad_top(card_cont_, pad, 0);
    lv_obj_set_style_pad_bottom(card_cont_, pad, 0);

    // 几何驱动焦点：滚动时实时把焦点同步到「中心最近卡片」
    // （拖动/惯性/snap 吸附全程跟随，消灭「区域内的卡片未选中」的割裂）
    lv_obj_add_event_cb(card_cont_, onScroll, LV_EVENT_SCROLL, NULL);
    lv_obj_add_event_cb(card_cont_, onScrollBegin, LV_EVENT_SCROLL_BEGIN, NULL);
    lv_obj_add_event_cb(card_cont_, onScrollEnd, LV_EVENT_SCROLL_END, NULL);

    // 初始化完成
    initialized_ = true;
}

void FactoryPages::Deinitialize() {
    if (!initialized_) return;

    if (card_cont_) lv_obj_delete(card_cont_);  // 删除容器（连带所有卡片）
    card_cont_ = nullptr;

    if (select_zone_) lv_obj_delete(select_zone_);  // 删除选择区域色块
    select_zone_ = nullptr;

    main_scr_       = nullptr;
    focused_idx_    = -1;
    program_scroll_ = false;
    syncing_        = false;

    if (detail_group_) lv_group_delete(detail_group_);  // 防御：清理残留的详情页焦点组
    detail_group_ = nullptr;
    main_group_   = nullptr;

    items_.clear();
    initialized_ = false;
}

void FactoryPages::AddTest(
    const char*           title,
    const char*           icon,
    std::function<void()> on_start,
    std::function<void()> on_stop,
    bool                  keep_on_back
) {
    if (!initialized_) Initialize();  // 懒初始化

    size_t idx = items_.size();
    items_.push_back({title, std::move(on_start), std::move(on_stop), keep_on_back});

    // 创建期间暂时脱离默认组，防止卡片内部的 button 自动混入主焦点组
    lv_group_set_default(NULL);

    // ===== 卡片本体：lv_obj（扁平透明，选中高亮交给底下的色块） =====
    lv_obj_t* card = lv_obj_create(card_cont_);
    lv_obj_remove_style_all(card);                    // 裸对象：清除主题样式（含 focus 外框）
    lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);     // 可点击（选中它）
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);  // 卡片自身不滚动
    lv_obj_set_size(card, LV_PCT(70), CARD_H);        // 卡片宽度 / 高度
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);     // 内部：图标-名称-进入按钮
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(card, 8, 0);  // 三元素间距
    lv_obj_set_style_pad_left(card, 12, 0);   // 左右内边距（元素不贴卡片边缘）
    lv_obj_set_style_pad_right(card, 12, 0);

    // 选中时文字变大：text_font 是可继承属性，title/icon label 自动跟随，失焦自动还原
    lv_obj_set_style_text_font(card, &lv_font_montserrat_20, LV_STATE_FOCUS_KEY);

    // 图标（可选）
    if (icon && icon[0]) {
        lv_obj_t* icon_lbl = lv_label_create(card);
        lv_label_set_text(icon_lbl, icon);
    }

    // 名称（占据剩余空间，把进入按钮推到最右）
    lv_obj_t* label = lv_label_create(card);
    lv_label_set_text(label, items_[idx].title.c_str());
    lv_obj_set_flex_grow(label, 1);

    // ===== 进入按钮：只有图标的 button（平时透明无边框，按下高亮，未选中不显示） =====
    lv_obj_t* btn = lv_button_create(card);
    lv_obj_set_size(btn, ENTER_BTN_WH, ENTER_BTN_WH);
    lv_obj_t* sym = lv_label_create(btn);
    lv_label_set_text(sym, LV_SYMBOL_RIGHT);
    lv_obj_center(sym);
    // 扁平：常态全透明无边框无阴影
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_text_color(btn, lv_color_hex(ENTER_SYM_COLOR), 0);  // 覆盖主题白字
    // 按下高亮：蓝色块 + 白色图标
    lv_obj_set_style_bg_color(btn, lv_color_hex(ZONE_COLOR), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_STATE_PRESSED);
    lv_obj_set_style_text_color(btn, lv_color_white(), LV_STATE_PRESSED);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);  // 未选中不显示
    lv_obj_add_event_cb(btn, onEnterClicked, LV_EVENT_CLICKED, (void*)(intptr_t)idx);

    // 事件注册（顺序：先注册回调再入组，保证首帧 FOCUSED 可被处理）
    lv_obj_add_event_cb(card, onCardClicked, LV_EVENT_CLICKED, (void*)(intptr_t)idx);  // 多余操作
    // lvgl 的 flex 对象默认就是点击选中
    
    lv_obj_add_event_cb(card, onCardFocused, LV_EVENT_FOCUSED, (void*)(intptr_t)idx);
    lv_obj_add_event_cb(card, onCardKey, LV_EVENT_KEY, (void*)(intptr_t)idx);

    // 墓碑化：卡片被删（任何路径）时同步失效 items_ 中的指针，防止悬垂访问
    lv_obj_add_event_cb(card, onCardDelete, LV_EVENT_DELETE, (void*)(intptr_t)idx);

    lv_group_set_default(main_group_);  // 恢复默认组（后续 AddTest 依赖）

    // 先登记指针再入组：首卡入组会触发 refocus → FOCUSED 事件（回调需读取 card/enter_btn）
    items_[idx].card      = card;
    items_[idx].enter_btn = btn;

    lv_group_add_obj(main_group_, card);  // lv_obj 不会自动入组，手动加入
}

void FactoryPages::onCardFocused(lv_event_t* e) {
    FactoryPages& self = GetInstance();
    size_t        idx  = (size_t)(intptr_t)lv_event_get_user_data(e);
    if (idx >= self.items_.size()) return;

    // 幂等保护：lv_group_focus_obj 对已是焦点的对象仍会重发 FOCUSED
    if (self.focused_idx_ == (int32_t)idx) return;
    // 滚动同步引起的聚焦：卡片就在中心，不能再反向触发滚动，否则循环
    if (self.syncing_) {
        self.applyFocusVisual(idx);
        return;
    }

    self.applyFocusVisual(idx);

    // 滚动居中（配合 snap center 吸附）
    // 程序动画置位：动画期间 SCROLL 帧回调跳过焦点同步，防止焦点被中途抢回
    self.program_scroll_ = true;
    lv_obj_scroll_to_view(self.items_[idx].card, LV_ANIM_ON);
}

void FactoryPages::onCardDelete(lv_event_t* e) {
    FactoryPages& self = GetInstance();
    size_t        idx  = (size_t)(intptr_t)lv_event_get_user_data(e);
    if (idx >= self.items_.size()) return;

    // 墓碑化：对象即将释放（此刻子对象尚存活，但我们无需触碰它们），
    // 置空指针后所有代码路径对该条目判空跳过；若它是当前选中卡则复位选中索引
    self.items_[idx].card      = nullptr;
    self.items_[idx].enter_btn = nullptr;
    if (self.focused_idx_ == (int32_t)idx) self.focused_idx_ = -1;
}

void FactoryPages::applyFocusVisual(size_t idx) {
    // 恢复上一次选中卡片的视觉（隐藏其进入按钮、清除键盘焦点状态）
    // 墓碑条目（对象已删）判空跳过
    if (focused_idx_ >= 0 && (size_t)focused_idx_ < items_.size() && (size_t)focused_idx_ != idx &&
        items_[focused_idx_].enter_btn) {
        lv_obj_add_flag(items_[focused_idx_].enter_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_state(items_[focused_idx_].card, LV_STATE_FOCUS_KEY);
    }
    focused_idx_ = (int32_t)idx;

    // 选中：进入按钮浮现 + 文字放大状态（与按钮同一处代码驱动，不依赖 indev 类型）
    // 注：LVGL 默认只在 KEYPAD/ENCODER indev 活跃时才加 LV_STATE_FOCUS_KEY（lv_obj.c L998），
    // 触摸路径不会加，故由我们显式管理；滚轮路径的重叠设置幂等无害
    lv_obj_clear_flag(items_[idx].enter_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_state(items_[idx].card, LV_STATE_FOCUS_KEY);
}

void FactoryPages::syncFocusToCenterCard() {
    if (items_.empty() || !card_cont_) return;

    // 找「卡片中心 y − 容器视口中心 y」绝对值最小的卡片（绝对坐标，滚动实时反映）
    lv_area_t cont_a;
    lv_obj_get_coords(card_cont_, &cont_a);
    int32_t cont_center = (cont_a.y1 + cont_a.y2) / 2;

    int32_t best_idx  = -1;
    int32_t best_dist = INT32_MAX;
    for (size_t i = 0; i < items_.size(); i++) {
        if (!items_[i].card) continue;
        lv_area_t card_a;
        lv_obj_get_coords(items_[i].card, &card_a);
        int32_t dist = (card_a.y1 + card_a.y2) / 2 - cont_center;
        if (dist < 0) dist = -dist;
        if (dist < best_dist) {
            best_dist = dist;
            best_idx  = (int32_t)i;
        }
    }
    if (best_idx < 0 || best_idx == focused_idx_) return;  // 幂等

    // 切焦点（触发 FOCUSED/DEFOCUSED → 视觉切换）；
    // syncing_ 抑制 onCardFocused 里的 scroll_to_view，几何→焦点单向同步，无循环
    syncing_ = true;
    lv_group_focus_obj(items_[best_idx].card);
    syncing_ = false;
}

void FactoryPages::onScroll(lv_event_t* e) {
    LV_UNUSED(e);
    FactoryPages& self = GetInstance();

    // 程序动画滚动中（滚轮/点击触发的 scroll_to_view）：跳过，动画终点即目标卡
    if (self.program_scroll_) return;

    self.syncFocusToCenterCard();  // 拖动/惯性/snap 吸附每帧跟随中心卡片
}

void FactoryPages::onScrollBegin(lv_event_t* e) {
    FactoryPages& self = GetInstance();

    if (lv_event_get_param(e) == NULL) {
        // 参数 NULL = 用户拖动 / raw 滚动：拖动接管，恢复帧同步；
        // 并杀掉 press 聚焦可能触发的 scroll_to_view 残留动画，避免与拖动打架
        self.program_scroll_ = false;
        lv_obj_stop_scroll_anim(self.card_cont_);
    }
    // 参数非 NULL = 动画滚动（scroll_to_view / 释放后 snap 吸附）：不处理
}

void FactoryPages::onScrollEnd(lv_event_t* e) {
    LV_UNUSED(e);
    FactoryPages& self = GetInstance();

    // 滚动结束：解除程序动画抑制，并兜底校正焦点（snap 已把某卡送到中心，幂等）
    self.program_scroll_ = false;
    self.syncFocusToCenterCard();
}

void FactoryPages::onCardClicked(lv_event_t* e) {
    FactoryPages& self = GetInstance();
    size_t        idx  = (size_t)(intptr_t)lv_event_get_user_data(e);
    if (idx >= self.items_.size()) return;

    // 主体点击只负责选中（幂等，已选中则无动作）；进入走进入按钮或 ENTER 键
    lv_group_focus_obj(self.items_[idx].card);
}

void FactoryPages::onEnterClicked(lv_event_t* e) {
    FactoryPages& self = GetInstance();
    size_t        idx  = (size_t)(intptr_t)lv_event_get_user_data(e);
    if (idx >= self.items_.size()) return;

    self.openDetail(idx);
}

void FactoryPages::onCardKey(lv_event_t* e) {
    FactoryPages& self = GetInstance();
    size_t        idx  = (size_t)(intptr_t)lv_event_get_user_data(e);
    if (idx >= self.items_.size()) return;

    uint32_t key = lv_event_get_key(e);
    if (key == LV_KEY_ENTER) {
        self.openDetail(idx);  // 实体按键 / 编码器按下 = 按下「进入按钮」
        return;
    }

    // 键盘方向键映射为焦点移动（滚轮/编码器由 group 层直接处理，无需此处）
    if (key == LV_KEY_UP || key == LV_KEY_LEFT) {
        lv_group_focus_prev(self.main_group_);  // 上一个焦点
    } else if (key == LV_KEY_DOWN || key == LV_KEY_RIGHT) {
        lv_group_focus_next(self.main_group_);  // 下一个焦点
    }
}

void FactoryPages::openDetail(size_t idx) {
    const TestItem& item = items_[idx];

    // 进入详情页前保存主界面滚动位置（返回时恢复）
    saved_scroll_y_ = lv_obj_get_scroll_y(card_cont_);

    // 详情页 = 新建一个 screen
    lv_obj_t* scr = lv_obj_create(NULL);

    // 详情页独立焦点组：让 Start/Stop/Back 只在本页内循环，不受主界面卡片干扰
    detail_group_ = lv_group_create();
    lv_group_set_default(detail_group_);  // 下面创建的按钮自动加入它

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

    // 「停止」按钮
    lv_obj_t* btn_stop = lv_button_create(scr);
    lv_obj_align(btn_stop, LV_ALIGN_CENTER, 40, 0);
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
    lv_obj_add_event_cb(btn_back, onBack, LV_EVENT_CLICKED, (void*)(intptr_t)idx);

    lv_group_set_default(main_group_);  // 恢复默认组，避免后续 AddTest 的卡片误入
    setAllIndevGroup(detail_group_);    // 让按键/旋钮导航详情页

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
