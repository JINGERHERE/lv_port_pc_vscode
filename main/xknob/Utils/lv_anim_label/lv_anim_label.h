/**
 * @file lv_anim_label.h
 *
 */

#ifndef LV_ANIM_LABEL_H
#define LV_ANIM_LABEL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"

/* [移植改动] LVGL v9 把「对象结构体」与「对象类结构体」挪进了私有头：
 *   - struct _lv_obj_t        → core/lv_obj_private.h（公开的 lv_obj.h 不再包含它）
 *   - struct _lv_obj_class_t  → core/lv_obj_class_private.h
 * 而本控件需要 ① 以 lv_obj_t 作为 lv_anim_label_t 的首成员（下方结构体），
 *              ② 用 .constructor_cb / .instance_size / .base_class 初始化 lv_obj_class_t，
 * 两者都需要完整类型定义，故必须在结构体声明之前引入私有头。
 * 根 lvgl_private.h 是一揽子聚合头且无开关门（只有 include guard），
 * 这也是 v9 里写自定义控件的标准做法（内置控件各自 include 自己的 *_private.h）。
 * 注意：本 include 必须放在本文件、且在本结构体之前 —— 放在 .c 里无效，因为 .c 会先包含本头。
 */
#include "lvgl_private.h"

typedef struct {
    lv_obj_t obj;
    lv_obj_t * label_1;
    lv_obj_t * label_2;
    lv_obj_t * label_act;
    lv_dir_t enter_dir;
    lv_dir_t exit_dir;
    lv_anim_t a_enter;
    lv_anim_t a_exit;
    uint32_t duration;
    lv_anim_path_cb_t path_cb;
}lv_anim_label_t;

extern const lv_obj_class_t lv_anim_label_class;

lv_obj_t * lv_anim_label_create(lv_obj_t * parent);

void lv_anim_label_set_dir(lv_obj_t * obj, lv_dir_t dir);

void lv_anim_label_set_enter_dir(lv_obj_t * obj, lv_dir_t dir);

void lv_anim_label_set_exit_dir(lv_obj_t * obj, lv_dir_t dir);

void lv_anim_label_set_time(lv_obj_t * obj, uint32_t duration);

void lv_anim_label_set_path(lv_obj_t * obj, lv_anim_path_cb_t path_cb);

void lv_anim_label_add_style(lv_obj_t * obj, lv_style_t * style);

void lv_anim_label_set_custom_enter_anim(lv_obj_t * obj, const lv_anim_t * a);

void lv_anim_label_set_custom_exit_anim(lv_obj_t * obj, const lv_anim_t * a);

void lv_anim_label_push_text(lv_obj_t * obj, const char* txt);

lv_dir_t lv_anim_label_get_enter_dir(lv_obj_t * obj);

lv_dir_t lv_anim_label_get_exit_dir(lv_obj_t * obj);

uint32_t lv_anim_label_get_time(lv_obj_t * obj);

lv_anim_path_cb_t lv_anim_label_get_path(lv_obj_t * obj);

const char * lv_anim_label_get_text(lv_obj_t * obj);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_ANIM_LABEL_H*/
