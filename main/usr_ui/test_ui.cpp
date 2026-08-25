#include "test_ui.h"

#include "lvgl.h"

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