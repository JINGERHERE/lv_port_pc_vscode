#ifndef __RESOURCE_POOL
#define __RESOURCE_POOL

#include "app/Utils/PageManager/ResourceManager.h"
#include "lvgl.h"

/* [移植改动] 本文件由 X-Knob 的 app/Resources/ResourcePool.h 移植而来。
 *
 * 原工程用 IMPORT_FONT/IMPORT_IMG 宏把 5 个预生成位图字体与约 40 张图片的
 * 编译期描述符（lv_font_t / lv_img_dsc_t）注册进资源池。LVGL v9 的资源二进制
 * 格式已变（lv_image_dsc_t 头部新增 magic/stride，cf 改用 lv_color_format_t，
 * v8 的 LV_IMG_CF_* 全部作废），且 PC 模拟器可直接从磁盘加载，故：
 *   - 字体：运行时用 Tiny TTF 从 bahnschrift.ttf 生成
 *   - 图片：运行时拼接 "A:<assets>/<name>.png" 路径，由 lodepng 解码
 *
 * 对外接口形态保持不变，使 MenuView / TemplateView 等调用点零改动。
 */
class ResourcePool
{

public:
    ResourceManager Font_;
    /* [移植改动] 原为一个 Image_ 资源池（预注册编译期图片描述符）。
     *            PC 端改为运行时按路径加载，无需预注册，故停用该成员。
     * 原: ResourceManager Image_;
     */

public:
    void Init();

    lv_font_t* GetFont(const char* name)
    {
        return (lv_font_t*)Font_.GetResource(name);
    }

    /* [移植改动] 语义变化：
     *   原 —— 返回编译进固件的 lv_img_dsc_t 指针；
     *   现 —— 返回 "A:<assets>/<name>.png" 路径字符串（供 lv_image_set_src 使用）。
     * 返回值类型仍为 const void*，以便调用点保持原样。 */
    const void* GetImage(const char* name);
};

extern ResourcePool Resource;

#endif
