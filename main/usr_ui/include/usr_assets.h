#pragma once

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// 获取图片资源源指针：模拟器返回 "A:" 路径字符串；ESP 返回 mmap 数据指针
// 用法: lv_image_set_src(img, usr_find_asset("esp_logo.png"));
void find_lv_assets(lv_obj_t* obj, const char* name);

#ifdef __cplusplus
}
#endif
