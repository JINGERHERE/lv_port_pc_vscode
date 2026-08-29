#include "usr_assets.h"

// ===== ===== ===== ===== ===== 平台标志 ===== ===== ===== ===== =====
// 1 = 模拟器(Windows/SDL)：资源以 "A:" 文件路径访问（LVGL 走 lodepng 解码）
// 0 = ESP(DemoProject)：资源经 esp_mmap_assets 映射，以数据指针访问
// 移植到 DemoProject 时改为 0
#define USR_UI_SIMULATOR 1

#if USR_UI_SIMULATOR

// 模拟器：拼接 "A:" 驱动路径，LVGL 的 lv_image_set_src 会对路径做拷贝，静态缓冲安全
const void* usr_find_asset(const char* name) {
    // USR_ASSETS_PREFIX 由 usr_ui/CMakeLists.txt 编译期注入（仓库根绝对路径）
    // "A:" 是 lv_conf.h 注册的 stdio 后端盘符，lv_fs 剥掉它后按绝对路径 fopen
    static char path[128];
    lv_snprintf(path, sizeof(path), "A:" USR_ASSETS_PREFIX "%s", name);
    return path;
}

#else

// ESP：StorageAssets::Find 命中 mmap 分区中的资源指针
#include "sel_board.h"

const void* usr_find_asset(const char* name) {
    const StorageAssets::Asset* a = board_hws->storage_assets->Find(name);
    return a ? a->data : nullptr;
}

#endif
