#include "display_xknob.h"

#include <lvgl.h>
#include <src/font/lv_symbol_def.h>

/* [移植改动] display_xknob() 是 X-Knob UI 的对外入口，作用等同原 X-Knob 工程
 * src/main.cpp 里的 App_Init()，也相当于 usr_ui 中的 test_ui()。
 * 调用方（main/src/main.c）由用户手动接入。
 *
 * 依赖链：display_xknob() -> App_Init()
 *           -> Resource.Init()          （资源池：Tiny TTF 字体 + "A:" 图片路径）
 *           -> PageManager.Install/Push （页面框架，进入 Pages/Menu）
 */
#include "app.h"

void display_xknob(void) {
    App_Init();
}
