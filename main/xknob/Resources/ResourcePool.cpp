#include "ResourcePool.h"

/* [移植改动] 本文件由 X-Knob 的 app/Resources/ResourcePool.cpp 移植而来。
 * 原实现整段以注释形式保留在下方（不删除），当前实现改为 PC 模拟器的运行时加载方案。
 */

ResourcePool Resource;

/* ===== [移植改动] 以下为原 ResourcePool.cpp 的资源注册段，保留备查 =====
 *
 * extern "C" {
 * #define IMPORT_FONT(name) \
 * do{\
 *     LV_FONT_DECLARE(font_##name)\
 *     Resource.Font_.AddResource(#name, (void*)&font_##name);\
 * }while(0)
 *
 * #define IMPORT_IMG(name) \
 * do{\
 *     LV_IMG_DECLARE(img_src_##name)\
 *     Resource.Image_.AddResource(#name, (void*)&img_src_##name);\
 * }while (0)
 *
 *     static void Resource_Init()
 *     {
 *         // Import Fonts
 *         IMPORT_FONT(bahnschrift_13);
 *         IMPORT_FONT(bahnschrift_17);
 *         IMPORT_FONT(bahnschrift_32);
 *         IMPORT_FONT(bahnschrift_65);
 *         IMPORT_FONT(agencyb_36);
 *
 *         // Import Images
 *         IMPORT_IMG(alarm);        IMPORT_IMG(battery);      IMPORT_IMG(battery_info);
 *         IMPORT_IMG(bicycle);      IMPORT_IMG(compass);      IMPORT_IMG(gps_arrow_default);
 *         IMPORT_IMG(gps_arrow_dark); IMPORT_IMG(gps_arrow_light); IMPORT_IMG(gps_pin);
 *         IMPORT_IMG(gyroscope);    IMPORT_IMG(locate);       IMPORT_IMG(map_location);
 *         IMPORT_IMG(menu);         IMPORT_IMG(origin_point); IMPORT_IMG(pause);
 *         IMPORT_IMG(satellite);    IMPORT_IMG(sd_card);      IMPORT_IMG(start);
 *         IMPORT_IMG(stop);         IMPORT_IMG(storage);      IMPORT_IMG(system_info);
 *         IMPORT_IMG(time_info);    IMPORT_IMG(trip);
 *         // IMPORT_IMG(arm);
 *         IMPORT_IMG(bluetooth);    IMPORT_IMG(switches);     IMPORT_IMG(dialpad);
 *         // IMPORT_IMG(miku);
 *         // IMPORT_IMG(circle_blue);
 *         IMPORT_IMG(dot_blue);     IMPORT_IMG(macos);
 *
 *         // HASS
 *         IMPORT_IMG(home);         IMPORT_IMG(home_fan);     IMPORT_IMG(home_bulb);
 *         IMPORT_IMG(home_air_cond); IMPORT_IMG(home_wash_machine);
 *
 *         // Setting
 *         IMPORT_IMG(setting);      IMPORT_IMG(setting_wifi); IMPORT_IMG(setting_timer);
 *         IMPORT_IMG(setting_brightness);
 * }
 *
 * }  // extern "C"
 * ======================================================================= */

void ResourcePool::Init()
{
    lv_obj_remove_style_all(lv_scr_act());

    /* [移植改动] LVGL v9 已删除 lv_disp_set_bg_color()，且六份 api_map 均无映射。
     *            改为直接对当前屏幕对象设置背景色与不透明度。
     * 原: lv_disp_set_bg_color(lv_disp_get_default(), lv_color_black());
     */
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, LV_PART_MAIN);

    Font_.SetDefault((void*)&lv_font_montserrat_14);

    /* [移植改动] 原为 Resource_Init() 注册 5 个预生成位图字体（v9 格式不兼容）。
     *            现改为运行时用 Tiny TTF 从 Resources/Font/bahnschrift.ttf 生成。
     * 依赖：LV_USE_TINY_TTF=1 且 LV_TINY_TTF_FILE_SUPPORT=1（已在 lv_conf.h 开启）。
     * 注意：Tiny TTF 是堆对象，必须只创建一次并常驻，不可反复创建（会泄漏）。
     */
    lv_font_t* font13 = lv_tiny_ttf_create_file("A:" USR_ASSETS_PREFIX "Font/bahnschrift.ttf", 13);
    lv_font_t* font17 = lv_tiny_ttf_create_file("A:" USR_ASSETS_PREFIX "Font/bahnschrift.ttf", 17);

    /* 文件缺失时 lv_tiny_ttf_create_file 返回 NULL，此处判空避免把 NULL 注册进池 */
    if (font13 != nullptr) Font_.AddResource("bahnschrift_13", font13);
    if (font17 != nullptr) Font_.AddResource("bahnschrift_17", font17);

    /* [移植改动] 以下 3 个字体现阶段未使用，暂不创建
     *            （每个 Tiny TTF 实例都占堆内存与字形缓存）。
     * 原: IMPORT_FONT(bahnschrift_32);
     * 原: IMPORT_FONT(bahnschrift_65);
     * 原: IMPORT_FONT(agencyb_36);
     * 需要时放开如下写法，并确保 Resources/Font/AGENCYB.TTF 已就位：
     *   Font_.AddResource("bahnschrift_32",
     *       lv_tiny_ttf_create_file("A:" USR_ASSETS_PREFIX "Font/bahnschrift.ttf", 32));
     *   Font_.AddResource("bahnschrift_65",
     *       lv_tiny_ttf_create_file("A:" USR_ASSETS_PREFIX "Font/bahnschrift.ttf", 65));
     *   Font_.AddResource("agencyb_36",
     *       lv_tiny_ttf_create_file("A:" USR_ASSETS_PREFIX "Font/AGENCYB.TTF", 36));
     */
}

const void* ResourcePool::GetImage(const char* name)
{
    /* [移植改动] 原实现为 Image_.GetResource(name)：返回预注册的 lv_img_dsc_t 指针。
     *            现改为运行时拼接 "A:" 文件路径；LVGL 的 lodepng 解码器会按 PNG 头
     *            自动解码（lv_conf.h 中 LV_USE_LODEPNG=1）。
     * lv_image_set_src() 会对传入的路径字符串做拷贝，故使用静态缓冲是安全的。
     */
    static char path[192];
    lv_snprintf(path, sizeof(path), "A:" USR_ASSETS_PREFIX "Image/%s.png", name);
    return path;
}
