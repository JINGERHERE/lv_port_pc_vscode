/*
 * MIT License
 * Copyright (c) 2021 _VIFEXTech
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* [移植改动] 本文件由 X-Knob 的 app/app.cpp 移植而来。
 * 说明：
 *  - App_Init() 是整个 X-Knob UI 的装配入口，由 display_xknob() 调用
 *    （对应原工程的 src/main.cpp -> App_Init()）。
 *  - 原工程依赖 HAL / AccountSystem / StatusBar，PC 模拟器均已停用。
 *  - 页面按阶段放开：当前仅 Template + Menu。
 */

#include "app.h"
/* [移植改动] HAL 依赖 ESP32 硬件，PC 模拟器不启用
 * 原: #include "hal/hal.h"
 */
#include "Utils/PageManager/PageManager.h"
/* [移植改动] AccountSystem（消息框架）的生产者全部是被停用的 HAL，PC 端不启用
 * 原: #include "Accounts/Account_Master.h"
 */
#include "Resources/ResourcePool.h"
#include "Pages/AppFactory.h"
/* [移植改动] StatusBar 为死代码且依赖 Account/HAL，PC 端不启用
 * 原: #include "Pages/StatusBar/StatusBar.h"
 */

void App_Init()
{
    /* [移植改动] 改为堆分配、永不释放（immortal object）。
     *
     * 原因：LVGL v9 内置的 SDL 驱动在关窗时的执行顺序是
     *     SDL_Quit() → lv_deinit() → exit(0)
     * 即 LVGL 先被反初始化，随后 exit(0) 才触发 C++ 静态对象析构。
     * 而 X-TRACK 版的 ~PageManager() 会调 SetStackClear() 卸载页面，
     * 此刻 LVGL 已销毁，只能操作悬空指针 → LV_ASSERT_OBJ 失败
     * → LV_ASSERT_HANDLER 是 while(1)，进程卡死、窗口无法关闭。
     *
     * 对比：X-TRACK 自带的 PC 模拟器用 v8 的 win32drv，关窗后只让 main 的
     * while 循环退出、并且不调 lv_deinit()，所以同样的写法在那边不会触发。
     * 这是「后端退出语义」的差异，不是写法错误。
     *
     * 堆分配的对象不会被析构，进程退出时由 OS 回收，从根本上避免退出期调用 LVGL。
     * 注：全局 ResourcePool Resource 的析构只释放 std::vector、不碰 LVGL，无需处理。
     * 原: static AppFactory factory;
     * 原: static PageManager manager(&factory);
     */
    static AppFactory* factory = new AppFactory();
    static PageManager* manager = new PageManager(factory);

    /* [移植改动] AccountSystem 初始化依赖 HAL，PC 端不启用
     * 原: Accounts_Init();
     */

    /* [移植改动] X-TRACK 版的 PM_State 不再硬编码页面 root 的尺寸，
     *            改由「根默认样式」提供（见 X-TRACK USER/App/App.cpp:81-88）。
     *            这里照它的写法建一个 rootStyle：宽高 = 屏幕分辨率 + 黑底。
     *            若缺少这段，页面 root 会退回 lv_obj 的默认尺寸（非 240x240）。
     *            注意：rootStyle 必须 static（PageManager 只存指针，需长于页面生命周期）。
     */
    static lv_style_t rootStyle;
    lv_style_init(&rootStyle);
    lv_style_set_width(&rootStyle, LV_HOR_RES);
    lv_style_set_height(&rootStyle, LV_VER_RES);
    lv_style_set_bg_opa(&rootStyle, LV_OPA_COVER);
    lv_style_set_bg_color(&rootStyle, lv_color_black());
    manager->SetRootDefaultStyle(&rootStyle);

    Resource.Init();

    /* [移植改动] StatusBar 未移植
     * 原: StatusBar::Init(lv_layer_top());
     */

    /* [移植改动] 页面安装清单：按移植进度逐步放开。
     *             Install(className, appName)：
     *               className -> AppFactory::CreatePage() 用于匹配类名
     *               appName   -> 注册名，供 Push/Pop 使用
     */
    manager->Install("Template", "Pages/Template");
    manager->Install("Menu", "Pages/Menu");

    /* [移植改动] 以下页面依赖 AccountSystem / HAL / WiFi / MQTT，暂未移植
     * 原: manager.Install("Startup",  "Pages/Startup");
     * 原: manager.Install("Playground", "Pages/Playground");
     * 原: manager.Install("SurfaceDial", "Pages/SurfaceDial");
     * 原: manager.Install("Hass",  "Pages/Hass");
     * 原: manager.Install("Setting", "Pages/Setting");
     * 原: manager.Install("WiFi", "Pages/WiFi");   // 原工程亦无 Pages/WiFi 目录，属死代码
     * 原: // manager.Install("Scene3D", "Pages/Scene3D");   // 上游原本即注释状态
     */

    manager->SetGlobalLoadAnimType(PageManager::LOAD_ANIM_OVER_TOP, 500);

    /* [移植改动] 原工程开机进入 StartUp 页（log 动画后自动跳 Menu）；StartUp 尚未移植，
     *            暂时直接进入 Menu。待 StartUp 移植完成后改回。
     * 原: manager.Push("Pages/Startup");
     */
    manager->Push("Pages/Menu");

    INIT_DONE();
}

void App_UnInit()
{
    /* [移植改动] 原实现为向 AccountSystem 发送保存/停止命令，依赖已停用的 HAL
     * 原: // ACCOUNT_SEND_NOTIFY_CMD(SysConfig, SYSCONFIG_CMD_SAVE);
     * 原: // ACCOUNT_SEND_NOTIFY_CMD(Storage, STORAGE_CMD_SAVE);
     * 原: // ACCOUNT_SEND_NOTIFY_CMD(Recorder, RECORDER_CMD_STOP);
     */
}
