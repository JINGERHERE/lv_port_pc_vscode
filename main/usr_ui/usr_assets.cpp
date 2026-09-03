#include "usr_assets.h"

// ===== ===== ===== ===== ===== 平台标志 ===== ===== ===== ===== =====
// 1 = 模拟器(Windows/SDL)：资源以 "A:" 文件路径访问（LVGL 走 lodepng 解码）
// 0 = ESP(DemoProject)：资源经 esp_mmap_assets 映射，以数据指针访问
// 移植到 DemoProject 时改为 0
#define USR_UI_SIMULATOR 1

#if USR_UI_SIMULATOR

// 模拟器：拼接 "A:" 驱动路径，LVGL 的 lv_image_set_src 会对路径做拷贝，静态缓冲安全
void find_lv_assets(lv_obj_t* obj, const char* name) {
    // USR_ASSETS_PREFIX 由 usr_ui/CMakeLists.txt 编译期注入（仓库根绝对路径）
    // "A:" 是 lv_conf.h 注册的 stdio 后端盘符，lv_fs 剥掉它后按绝对路径 fopen
    char path[128];
    lv_snprintf(path, sizeof(path), "A:" USR_ASSETS_PREFIX "%s", name);

    lv_image_set_src(obj, path);
}

#else

// usr_assets.cpp 的 ESP32 分支
static mmap_assets_handle_t                  s_handle;
static std::map<std::string, lv_image_dsc_t> s_dscs;  // 键值直接是 LVGL 消费形态

static void ensure_init() {
    if (s_handle) return;
    mmap_assets_config_t cfg = {.partition_label = "storage", /* max_files/checksum 见下文 */};
    mmap_assets_new(&cfg, &s_handle);
    int count = mmap_assets_get_stored_files(s_handle);
    for (int i = 0; i < count; i++) {
        lv_image_dsc_t dsc = {};
        dsc.header.cf      = LV_COLOR_FORMAT_RAW;               // 首字节 <0x20 → 判为 VARIABLE
        dsc.data           = mmap_assets_get_mem(s_handle, i);  // mmap 指针，天然持久
        dsc.data_size      = mmap_assets_get_size(s_handle, i);
        dsc.header.w       = mmap_assets_get_width(s_handle, i);  // 打包时已解析，白送的
        dsc.header.h       = mmap_assets_get_height(s_handle, i);
        s_dscs[mmap_assets_get_name(s_handle, i)] = dsc;
    }
}

void find_lv_assets(lv_obj_t* obj, const char* name) {
    ensure_init();  // 就是你说的"判断有没有初始化"
    auto it = s_dscs.find(name);
    if (it != s_dscs.end()) lv_image_set_src(obj, &it->second);
}

#endif
