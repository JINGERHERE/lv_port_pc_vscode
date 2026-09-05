#include "test_ui.h"

#include <lvgl.h>
#include <src/font/lv_symbol_def.h>

#include "usr_assets.h"

static void set_x_cb(void* var, int32_t v) {
    lv_obj_set_x((lv_obj_t*)var, v);
}

static lv_obj_t* g_list = NULL;

// 返回后删除页面
static void panel_del_cb(lv_anim_t* a) {
    // 删除页面
    lv_obj_delete((lv_obj_t*)a->var);  // lv_anim_t 中的 var 指向的是 panel 对象
    // var 类型为 void*，需要强制类型转换为 lv_obj_t*

    // 保存列表当前的滚动位置
    int32_t scroll_y = 0;
    if (g_list) scroll_y = lv_obj_get_scroll_y(g_list);

    // 恢复列表滚动位置
    if (g_list) lv_obj_scroll_to_y(g_list, scroll_y, LV_ANIM_OFF);
}

static void panel_back_cb(lv_event_t* e) {
    lv_obj_t* panel = (lv_obj_t*)lv_event_get_user_data(e);
    int32_t   h_res = lv_display_get_horizontal_resolution(lv_display_get_default());

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, panel);
    lv_anim_set_exec_cb(&a, set_x_cb);
    lv_anim_set_values(&a, 0, h_res);
    lv_anim_set_duration(&a, 200);
    lv_anim_set_path_cb(&a, lv_anim_path_custom_bezier3);
    LV_ANIM_SET_EASE_OUT_CUBIC(&a);
    lv_anim_set_completed_cb(&a, panel_del_cb);
    lv_anim_start(&a);
}

static void btn0_ui(lv_event_t* e) {
    lv_obj_t* scr   = lv_screen_active();
    int32_t   h_res = lv_display_get_horizontal_resolution(lv_display_get_default());
    int32_t   v_res = lv_display_get_vertical_resolution(lv_display_get_default());

    // 新页面
    lv_obj_t* panel = lv_obj_create(scr);
    lv_obj_set_size(panel, h_res * 0.6, v_res * 0.6);
    // lv_obj_set_pos(panel, h_res, 0);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xADD8E6), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, LV_PART_MAIN);
    // lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN);
    // lv_obj_add_flag(panel, LV_OBJ_FLAG_FLOATING);
    // lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(panel);

    // 页面文字
    lv_obj_t* title = lv_label_create(panel);
    lv_label_set_text(title, "WLAN Setting");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    // 返回按钮
    lv_obj_t* back_btn = lv_btn_create(panel);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_t* back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT " Back");
    lv_obj_center(back_label);

    lv_obj_add_event_cb(back_btn, panel_back_cb, LV_EVENT_CLICKED, panel);

    // 页面动画
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, panel);
    lv_anim_set_exec_cb(&a, set_x_cb);
    lv_anim_set_values(&a, h_res, 0);
    lv_anim_set_duration(&a, 300);
    lv_anim_set_path_cb(&a, lv_anim_path_custom_bezier3);  // 设置动画路径 // 自定义三次贝塞尔曲线
    // LV_ANIM_SET_EASE_OUT_CUBIC(&a); // 预设宏
    lv_anim_set_bezier3_param(&a, 338, 1024, 696, 1024);
    lv_anim_start(&a);
}

static void btn1_ui(lv_event_t* e) {
    lv_obj_t* scr = lv_screen_active();
    int32_t   h_res = lv_display_get_horizontal_resolution(lv_display_get_default());
    int32_t   v_res = lv_display_get_vertical_resolution(lv_display_get_default());

    lv_obj_t* panel = lv_obj_create(scr);
    lv_obj_set_size(panel, h_res * 0.6, v_res * 0.6);
    lv_obj_center(panel);

    // 页面内容
    lv_obj_t* img = lv_image_create(panel);
    find_lv_assets(img, "esp_logo.png");
    // lv_obj_center(img);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, -20);

    // 返回按钮
    lv_obj_t* back_btn = lv_btn_create(panel);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_t* back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT " Back");
    lv_obj_center(back_label);

    lv_obj_add_event_cb(back_btn, panel_back_cb, LV_EVENT_CLICKED, panel);

    // 页面动画
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, panel);
    lv_anim_set_exec_cb(&a, set_x_cb);
    lv_anim_set_values(&a, h_res, 0);
    lv_anim_set_duration(&a, 300);
    lv_anim_set_path_cb(&a, lv_anim_path_custom_bezier3);  // 设置动画路径 // 自定义三次贝塞尔曲线
    LV_ANIM_SET_EASE_OUT_CUBIC(&a); // 预设宏
    lv_anim_start(&a);
}

void test_ui(void) {
    lv_obj_t* scr = lv_scr_act();
    // lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    g_list = lv_list_create(scr);

    lv_obj_center(g_list);

    lv_list_add_text(g_list, "Item 1");
    lv_list_add_text(g_list, "Item 2");

    lv_obj_t* btn0 = lv_list_add_btn(g_list, LV_SYMBOL_WIFI, "WLAN");
    lv_obj_t* btn1 = lv_list_add_btn(g_list, LV_SYMBOL_OK, "OK");

    lv_list_add_text(g_list, "Item 3");

    lv_obj_t* btn2  = lv_list_add_btn(g_list, LV_SYMBOL_CLOSE, "CLOSE");
    lv_obj_t* btn3  = lv_list_add_btn(g_list, LV_SYMBOL_SETTINGS, "SETTINGS");
    lv_obj_t* btn4  = lv_list_add_btn(g_list, LV_SYMBOL_DRIVE, "DRIVE");
    lv_obj_t* btn5  = lv_list_add_btn(g_list, LV_SYMBOL_REFRESH, "REFRESH");
    lv_obj_t* btn6  = lv_list_add_btn(g_list, LV_SYMBOL_VOLUME_MID, "VOLUME_MID");
    lv_obj_t* btn7  = lv_list_add_btn(g_list, LV_SYMBOL_CUT, "CUT");
    lv_obj_t* btn8  = lv_list_add_btn(g_list, LV_SYMBOL_DUMMY, "DUMMY");
    lv_obj_t* btn9  = lv_list_add_btn(g_list, LV_SYMBOL_CHARGE, "CHARGE");
    lv_obj_t* btn10 = lv_list_add_btn(g_list, LV_SYMBOL_NEW_LINE, "NEW_LINE");
    lv_obj_t* btn11 = lv_list_add_btn(g_list, LV_SYMBOL_BACKSPACE, "BACKSPACE");

    lv_obj_add_event_cb(btn0, btn0_ui, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn1, btn1_ui, LV_EVENT_CLICKED, NULL);
}

#if 0

void my_gui() {
    lv_obj_t* switch_obj = lv_switch_create(lv_scr_act());
    lv_obj_set_size(switch_obj, 120, 60);
    lv_obj_align(switch_obj, LV_ALIGN_CENTER, 0, 0);
}

void round_gui() {
    lv_obj_t* scr = lv_scr_act();

    // 屏幕背景设黑，模拟圆形屏幕外的"不可见"区域
    lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);

    // 创建圆形容器，作为圆形屏幕的"可见区域"
    lv_obj_t* round_cont = lv_obj_create(scr);
    lv_obj_set_size(round_cont, 360, 360);
    lv_obj_center(round_cont);
    lv_obj_set_style_radius(round_cont, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_clip_corner(round_cont, true, LV_PART_MAIN);
    lv_obj_set_style_border_width(round_cont, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(round_cont, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(round_cont, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(round_cont, LV_OBJ_FLAG_SCROLLABLE);

    // 在圆形容器内放置 UI 控件
    lv_obj_t* btn = lv_btn_create(round_cont);
    lv_obj_set_size(btn, 100, 50);
    lv_obj_center(btn);
    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, "Hello");
    lv_obj_center(label);
}

void img_gui() {
    lv_obj_t* img = lv_image_create(lv_scr_act());
    find_lv_assets(img, "esp_logo.png");
    lv_obj_center(img);
}

#endif