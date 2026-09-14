# xknob —— X-Knob 旋钮 UI 移植

把开源项目 **X-Knob** 的 LVGL 旋钮界面移植到本模拟器工程（LVGL v9.5 / CMake / SDL2 / 无 FreeRTOS）。

---

## 1. 移植来源

本目录**有两个来源**——框架层跟踪 X-TRACK，页面层跟踪 X-Knob：

| 来源 | 本地路径 | 提供什么 |
|---|---|---|
| **X-TRACK** | `~/Github/References/X-TRACK/Software/X-Track/USER/App` | `Utils/PageManager/`（页面框架）、`Utils/ResourceManager/`（资源注册表） |
| **X-Knob** | `~/Github/References/X-Knob/1.Firmware/src/app` | `Pages/`（Menu、_Template）、`Configs/`、`Resources/`、`Utils/lv_ext/`、`app.h`/`app.cpp` |
| 字体原素材 | `~/Github/References/X-TRACK/ArtDesign/` | `bahnschrift.ttf`、`AGENCYB.TTF` |

| 项 | 值 |
|---|---|
| 源 LVGL 版本 | X-TRACK **v8.3** ／ X-Knob **v8.1**（ESP32-S3 + Arduino + PlatformIO） |
| 目标 LVGL 版本 | **v9.5**（本工程根目录 `lvgl/`） |
| 移植范围 | **仅 UI**。放弃：电机 / 力反馈 / WiFi / MQTT / BLE / 电源管理 / SD 卡 / 消息框架 |

> **为什么 PageManager 取 X-TRACK 而不是 X-Knob**：两者同源，但 X-Knob 做了裁剪（删掉了 `Replace`、`SetRootDefaultStyle`、`onViewUnload` 钩子）并改了成员命名。X-TRACK 版最完整，且在 X-Knob 版上补齐这三样要动的面更大，故直接以 X-TRACK 版为准。

---

## 2. 关于架构：X-Knob 是 X-TRACK 框架的一层

参考项目之间的血缘关系（这也是本目录结构设计的依据）：

```
X-TRACK（LVGL v8，AT32 自行车码表）
   └── 页面框架 PageManager + 消息框架 DataCenter + 各业务页面
        │
        ├──► X-Knob：直接拿来 PageManager，换上「旋钮屏」的 UI 与 AccountSystem
        │            ← 本目录的页面层来自这一支
        │
        └──► Peak：同一框架移植到 ESP32（超迷你智能小终端）
```

换句话说，**X-Knob 的这套 PageManager 框架 = X-TRACK 框架去掉业务层、换成旋钮 UI 后的产物**。因此本工程把 **`main/xknob/` 本身当作框架根**——将来要接 X-TRACK 的其余页面（表盘、系统信息、地图等）时，可以直接往这个根里加。

---

## 3. 目录结构

```
main/xknob/
├── CMakeLists.txt            独立静态库目标 xknob
├── README.md                 本文档
├── display_xknob.h/.cpp      UI 对外入口 display_xknob()（本工程自建）
├── app.h  app.cpp            装配层：App_Init() 设根样式、安装页面并进入 Menu
├── Configs/Version.h         版本宏（Menu::Update() 使用）
├── Utils/
│   ├── PageManager/          页面框架（10 文件，来自 X-TRACK）
│   ├── ResourceManager/      资源注册表（2 文件，来自 X-TRACK）
│   └── lv_ext/               LVGL 扩展（6 文件，来自 X-Knob）
├── Resources/
│   ├── ResourcePool.h/.cpp   资源池：字体 + 图片
│   ├── Font/                 bahnschrift.ttf、AGENCYB.TTF
│   └── Image/                图标 PNG（6 张英文名在用 + 20 张 X-TRACK 素材）
├── Pages/
│   ├── Page.h  AppFactory.h/.cpp
│   ├── Menu/                 旋钮主界面（6 文件）
│   └── _Template/            空白模板页（6 文件）
└── tools/
    └── v8img2png.py          离线工具：LVGL v8 图片 .c 数组 → PNG
```

**共 40 个源文件。**

**include 前缀规则**：以 `main/xknob/` 为 include 根，所以上游的相对路径原样保留（只去掉一层 `app/`）：

| 上游写法 | 移植后 |
|---|---|
| `#include "app/Utils/PageManager/PageManager.h"` | `#include "Utils/PageManager/PageManager.h"` |
| `#include "app/Resources/ResourcePool.h"` | `#include "Resources/ResourcePool.h"` |
| `#include "app/Configs/Version.h"` | `#include "Configs/Version.h"` |
| `#include "app/app.h"` | `#include "app.h"` |
| `#include "../Page.h"` | 不变（目录层级未变） |

---

## 4. 逐文件移植清单

「改动」列：**原样**（逐字节相同）/ **小改** / **重写**。

### 4.1 页面框架 `Utils/PageManager/`（10 文件，来源 **X-TRACK**）

| 来源 | 改动 | 说明 |
|---|---|---|
| `Utils/PageManager/PageManager.h` | 原样 | 页面栈对外接口，含 `Replace` / `SetRootDefaultStyle` |
| `Utils/PageManager/PageBase.h` | 小改 | 补 `<string.h>`（集中覆盖本目录 4 个 .cpp 的用量） |
| `Utils/PageManager/PageBase.cpp` | 小改 | `lv_mem_free` → `lv_free`（X-Knob 版把本文件内容内联进了头文件，故这是**新增文件**） |
| `Utils/PageManager/PageFactory.h` | 原样 | 抽象工厂 |
| `Utils/PageManager/PM_Anim.cpp` | 原样 | 转场动画参数表 |
| `Utils/PageManager/PM_Base.cpp` | 原样 | 页面池 / 页面栈 / 装配 API |
| `Utils/PageManager/PM_Router.cpp` | 小改 | `lv_mem_alloc` → `lv_malloc`。本文件含 `Replace`/`Push`/`Pop`/`BackHome`/`SwitchTo` |
| `Utils/PageManager/PM_State.cpp` | 小改 | `lv_mem_free` → `lv_free` |
| `Utils/PageManager/PM_Drag.cpp` | 小改 | `lv_event_get_current_target` → `..._obj`；`lv_event_send` → `lv_obj_send_event`；补 `<algorithm>`/`<cstdlib>` |
| `Utils/PageManager/PM_Log.h` | 原样 | ARDUINO 分支定义成空宏，**完全不依赖 `<Arduino.h>`** |

### 4.2 资源注册表 `Utils/ResourceManager/`（2 文件，来源 **X-TRACK**）

| 来源 | 改动 | 说明 |
|---|---|---|
| `Utils/ResourceManager/ResourceManager.h` | 原样 | 通用 name→ptr 注册表 |
| `Utils/ResourceManager/ResourceManager.cpp` | 原样 | 已自带 `<string.h>`，用 `LV_LOG_*` 打日志，**无需 v9 补丁** |

> X-Knob 当年把这两个文件平移进了 `Utils/PageManager/` 并把日志宏从 `LV_LOG_*` 换成 `PM_LOG_*`。本次按 X-TRACK 的布局**恢复到独立目录**，并同步修正了引用它的 `Resources/ResourcePool.h`。
> 注：X-TRACK 里它叫 `Utils/ResourceManager/`，X-Knob 里叫 `Utils/PageManager/ResourceManager.{h,cpp}`；`.h` 两边完全相同。

### 4.3 LVGL 扩展 `Utils/lv_ext/`（6 文件，来源 **X-Knob**）

| 来源 | 改动 | 说明 |
|---|---|---|
| `Utils/lv_ext/lv_obj_ext_func.h` | 原样 | 含 `lv_get_indev()` 等扩展 |
| `Utils/lv_ext/lv_obj_ext_func.cpp` | 小改 | `indev->driver->type` → `lv_indev_get_type()`；自带 `<string.h>` |
| `Utils/lv_ext/lv_anim_timeline_wrapper.h/.c` | 原样 | 时间轴包装宏 |
| `Utils/lv_ext/lv_label_anim_effect.h/.cpp` | 原样 | 数字滚动切换动画。上游全仓无引用，按要求一并搬入备用 |

### 4.4 页面 `Pages/`（来源 **X-Knob**）

| 来源 | 改动 | 说明 |
|---|---|---|
| `Pages/Page.h` | 小改 | 注释掉 `StatusBar.h` 的 include（StatusBar 未移植） |
| `Pages/AppFactory.h` | 小改 | 补 `#pragma once`（原头文件无 include guard） |
| `Pages/AppFactory.cpp` | 小改 | 注释掉 6 个未移植页面的 include 及其中 5 个的 `APP_CLASS_MATCH`；补 `<string.h>` |
| `Pages/Menu/Menu.h` | 原样 | — |
| `Pages/Menu/Menu.cpp` | 小改 | 见 §5.1、§5.2；成员访问改为 `_root`/`_Manager` |
| `Pages/Menu/MenuView.h` | 小改 | 保持 `#include "../Page.h"`，加说明注释 |
| `Pages/Menu/MenuView.cpp` | 小改 | 修正上游一处字符串拼接笔误，见 §5.3 |
| `Pages/Menu/MenuModel.h/.cpp` | 重写 | 退化为空壳，见 §5.3 |
| `Pages/_Template/Template.h` | 原样 | — |
| `Pages/_Template/Template.cpp` | 小改 | 去 `<Arduino.h>`；`timer->user_data`、`lv_event_get_target` 改写；成员访问改为 `_root`/`_Name`/`_Manager` |
| `Pages/_Template/TemplateView.h` | 原样 | — |
| `Pages/_Template/TemplateView.cpp` | 小改 | `montserrat_10` → `montserrat_14`；占位图 `macos` → `esp_logo` |
| `Pages/_Template/TemplateModel.h/.cpp` | 原样 | — |
| `Configs/Version.h` | 原样 | `Menu::Update()` 依赖其 `VERSION_*` 宏 |

### 4.5 资源与装配

| 目标 | 改动 | 说明 |
|---|---|---|
| `Resources/ResourcePool.h` | 重写 | 去掉 `Image_` 成员；`GetImage()` 改为声明；include 改指 `Utils/ResourceManager/` |
| `Resources/ResourcePool.cpp` | 重写 | 编译期注册 → 运行时加载，见 §5.3 |
| `app.h` | 重写 | 只保留三个模式枚举 + `offsetof`/`container_of` 宏，见 §5.2 |
| `app.cpp` | 重写 | 见 §5.4（根样式 + immortal object） |
| `display_xknob.h/.cpp` | **新增** | 本工程 UI 入口，调用 `App_Init()` |
| `tools/v8img2png.py` | **新增** | v8 图片 `.c` → PNG |
| `Resources/Font/*`、`Resources/Image/*` | **新增** | 字体与图标 |

---

## 5. 改动说明

### 5.1 LVGL v8 → v9.5 API 改写（原写法一律以注释保留）

| 原写法（v8） | 现写法（v9.5） | 涉及文件 |
|---|---|---|
| `lv_mem_alloc` / `lv_mem_free` | `lv_malloc` / `lv_free` | `PM_Router.cpp`、`PM_State.cpp`、`PageBase.cpp` |
| `lv_event_get_current_target()` | `lv_event_get_current_target_obj()` | `PM_Drag.cpp` |
| `lv_event_send(obj, ...)` | `lv_obj_send_event(obj, ...)` | `PM_Drag.cpp` |
| `lv_event_get_target()` | `lv_event_get_target_obj()` | `Menu.cpp`、`Template.cpp` |
| `indev->driver->type` | `lv_indev_get_type(indev)` | `lv_obj_ext_func.cpp` |
| `timer->user_data` | `lv_timer_get_user_data(timer)` | `Menu.cpp`、`Template.cpp` |
| `lv_disp_set_bg_color(...)` | `lv_obj_set_style_bg_color(lv_screen_active(), ...)` | `ResourcePool.cpp` |
| `lv_meter_*` | v9 已移除，需改 `lv_scale_*`（尚未涉及） | — |

> v9.5 的 `lvgl.h` 会默认包含 `lv_api_map_v8.h`~`v9_4`（仅当定义了 `LV_DISABLE_API_MAPPING` 时才不包含；本工程未定义），因此 `lv_obj_del`、`lv_scr_act`、`lv_group_del`、`lv_img_set_src`、`lv_anim_set_time` 等大量 v8 名称**无需修改**即可编译。上表是兼容层仍未覆盖、必须手改的部分。

### 5.2 剥离 Arduino / ESP32 平台依赖（注释保留，不删除）

| 停用项 | 出现位置 |
|---|---|
| `#include <Arduino.h>` | `MenuModel.cpp`、`Template.cpp` |
| `#include "hal/motor.h"` | `Menu.cpp`、`app.h` |
| `HAL::power_off()` | `Menu.cpp`（仅此处） |
| `#include "hal/hal.h"`、`Accounts_Init()` | `app.cpp` |
| `#include "app/Accounts/Account_Master.h"` | `MenuModel.h`、`app.cpp` |
| `#include "app/Pages/StatusBar/StatusBar.h"` | `Page.h`、`app.cpp` |
| `Serial.printf` | 改为标准 `printf` |
| `ACCOUNT_SEND_NOTIFY_CMD`、`display_init()` | `app.h` |

### 5.3 重写与修正

| 文件 | 处理 |
|---|---|
| `Pages/Menu/MenuModel.{h,cpp}` | **退化为空壳**。原 `Init/Deinit/ChangeMotorMode` 依赖 AccountSystem 与电机 HAL，整段注释保留；本页 UI 并不消费电机数据 |
| `Resources/ResourcePool.cpp` | 由「编译期注册位图字体/图片描述符」改为「运行时加载」：字体走 Tiny TTF，图片走 `"A:"` 路径 + lodepng |
| `Pages/Menu/MenuView.cpp` | **修正上游笔误**：`"Surface Dial" "Control\n"` 两行相邻字面量被 C++ 拼接成 `"Surface DialControl\n"`，已补 `\n`（原写法以注释保留） |
| `app.h` | 删掉 AccountSystem 宏、Arduino 任务通知、`display_init` 声明 |

### 5.4 `app.cpp` 的两处关键设计

**① 根样式 `rootStyle`** —— X-TRACK 版的 `PM_State::StateLoadExecute` **不再硬编码页面 root 的尺寸**，改由 `manager.SetRootDefaultStyle()` 提供：

```cpp
static lv_style_t rootStyle;          // 必须 static：PageManager 只存指针
lv_style_init(&rootStyle);
lv_style_set_width(&rootStyle, LV_HOR_RES);
lv_style_set_height(&rootStyle, LV_VER_RES);
lv_style_set_bg_opa(&rootStyle, LV_OPA_COVER);
lv_style_set_bg_color(&rootStyle, lv_color_black());
manager->SetRootDefaultStyle(&rootStyle);
```

缺少这段，页面 root 会退回 lv_obj 的默认尺寸（不是 240×240）。

**② `manager` / `factory` 改为堆分配、永不析构（immortal object）** —— 这是为了绕开一个**退出期崩溃**：

```
关闭 SDL 窗口
  └─ lv_sdl_window.c  SDL_QUIT 分支
       ├─ SDL_Quit()
       ├─ lv_deinit()          ← ★ LVGL 先被反初始化
       └─ exit(0)              ← LV_SDL_DIRECT_EXIT=1
            └─ C++ 静态对象析构
                 └─ ~PageManager() → SetStackClear() → 操作已销毁的页面对象
                      └─ LV_ASSERT_OBJ 失败 → LV_ASSERT_HANDLER 是 while(1)
                           └─ 进程卡死、窗口关不掉
```

故改为：

```cpp
static AppFactory*  factory = new AppFactory();
static PageManager* manager = new PageManager(factory);
```

堆对象不会被析构，进程退出时由 OS 回收，从根本上避免退出期调用 LVGL。

> **这不是写法错误**：X-TRACK 自己的 `App.cpp:40-41` 用的是**逐字相同**的 `static AppFactory factory; static PageManager manager(&factory);`。差异在后端——X-TRACK 的 PC 模拟器用 v8 的 `lv_drivers/win32drv`，关窗后只让 main 的 `while` 退出、**且不调 `lv_deinit()`**，所以 `~PageManager()` 执行时 LVGL 仍然存活。这是「后端退出语义」的差异。
>
> 全工程唯一在退出期碰 LVGL 的就是这个析构器；全局 `ResourcePool Resource` 的析构只释放 `std::vector`，安全。

---

## 6. 构建配置

### 6.1 `main/xknob/CMakeLists.txt`

（以下为文件原文）

```cmake
# 自动添加源文件 # CONFIGURE_DEPENDS 构建时自动检测目录文件变化
# 移植改动：GLOB → GLOB_RECURSE，递归收集 Utils/ Pages/ Resources/ Configs/ 等子目录下的源码
# 原: file(GLOB USR_SOURCES CONFIGURE_DEPENDS *.c *.cpp)
file(GLOB_RECURSE USR_SOURCES CONFIGURE_DEPENDS *.c *.cpp)

# 编译成静态库
# 移除了原 usr_ui 模板中的 USR_PRIV_SOURCES / private/ 一段（本目录无 private/ 子目录）
add_library(xknob STATIC ${USR_SOURCES})

# 资源目录：编译期注入绝对路径，ResourcePool 运行时按 "A:<路径>" 加载
# 移植改动：资源由 assets/ 改到 Resources/{Image,Font}/ 下，故前缀指向 Resources/
target_compile_definitions(xknob PRIVATE
    USR_ASSETS_PREFIX="${CMAKE_CURRENT_SOURCE_DIR}/Resources/"
)

# 头文件目录：以本目录为 include 根
#   PRIVATE 侧无需单列——本目录即根，使 "Utils/..." "Pages/..." "Resources/..."
#   "Configs/..." "app.h" 以及各页面里的 "../Page.h" 全部可解析
#   PUBLIC 侧是必需的：main/src/main.c 通过 #include "display_xknob.h" 接入本库，
#   而 display_xknob.h 就在本目录下
target_include_directories(xknob PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 链接 LVGL 库
target_link_libraries(xknob PUBLIC lvgl)
```

新增源文件（如 `PageBase.cpp`）会被 `GLOB_RECURSE` 自动收集，**无需改 CMakeLists**。

### 6.2 根 `CMakeLists.txt`（接入 xknob 目标）

```cmake
add_subdirectory(${PROJECT_SOURCE_DIR}/main/xknob)  # 添加 xknob 子目录

target_link_libraries(main lvgl ... lv_demos_ext
                        usr_ui xknob)               # 追加 xknob
```

---

## 7. 资源

### 7.1 字体（Tiny TTF，运行时加载，无需 lv_font_conv）

| 文件 | 来源 | 用途 |
|---|---|---|
| `Resources/Font/bahnschrift.ttf` | X-TRACK `ArtDesign/` | `bahnschrift_13`、`bahnschrift_17`（Menu 在用） |
| `Resources/Font/AGENCYB.TTF` | X-TRACK `ArtDesign/` | `agencyb_36`（暂未启用） |

依赖配置：`lv_conf.h` 中 `LV_USE_TINY_TTF=1`、`LV_TINY_TTF_FILE_SUPPORT=1`、`LV_USE_FS_STDIO=1`（盘符 `'A'`）。
在 `ResourcePool::Init()` 中创建并常驻（Tiny TTF 是堆对象，不可反复创建）。

### 7.2 图片

`Resources/Image/` 下 **6 张英文名 PNG** 正在被代码引用：

| 输出 | 来源 `.c`（相对 X-Knob 的 `1.Firmware/src/`） | 尺寸 | 使用者 |
|---|---|---|---|
| `dialpad.png` | `app/Resources/Image/img_src_dialpad.c` | 42×42 | Menu |
| `switches.png` | `app/Resources/Image/img_src_switches.c` | 42×42 | Menu |
| `home.png` | `app/Resources/Image/img_src_home.c` | 42×42 | Menu |
| `system_info.png` | `app/Resources/Image/img_src_system_info.c` | 36×36 | Menu |
| `setting.png` | `app/Resources/Image/img_src_setting.c` | 42×42 | Menu |
| `esp_logo.png` | 本工程原有（非转换产物） | 96×96 | `_Template` 占位 |

转换命令（脚本解析 `.c` 中 `#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP == 0` 分支，按「RGB565 小端 2 字节 + A8 1 字节」逐像素还原为 RGBA PNG，仅用 Python 标准库）：

```bash
python3 main/xknob/tools/v8img2png.py <in.c> <out.png> [...]
```

> **另有 20 张中文名 PNG**（`指南针.png`、`电池信息.png`、`菜单.png` …），是从 X-TRACK `ArtDesign/` 拷来的素材，**文件名与代码请求的英文资源名对不上**（**上游 X-Knob 代码**请求的是 `compass`、`battery_info`、`menu` 这类英文名，对应关系如 指南针→compass）。当前 Menu/Template 的活代码只用到上面那 6 张，用不到它们；将来启用对应页面时需要改名或加一层映射。

---

## 8. 未移植内容（范围声明）

| 上游路径 | 文件数 | 未移植原因 |
|---|---|---|
| `Utils/AccountSystem/`（X-Knob）/ `Utils/DataCenter/`（X-TRACK） | 7 | 消息框架；数据生产者全是被放弃的 HAL。**两边是同一套设计的两种命名** |
| `Accounts/`（X-Knob）/ `Common/DataProc/`（X-TRACK） | 6 / 16 | 数据加工节点，依赖 HAL（X-TRACK 侧共 16 个文件，其中 13 个是 `DP_*` 数据节点） |
| `Pages/StatusBar/` | 2 | 死代码（调用点全被注释）且依赖 Account/HAL |
| `Pages/SystemInfos/` | 6 | 机器人关节/IMU，无引用 |
| `Pages/StartUp/` | 6 | 开机动画页 |
| `Pages/Playground/` | 6 | 依赖电机；含 `lv_meter`，需改 `lv_scale` |
| `Pages/Setting/` | 6 | 依赖 WiFi/NVS；含 `lv_meter` |
| `Pages/HASS/` | 6 | 依赖 MQTT |
| `Pages/SurfaceDial/` | 6 | 依赖 BLE + 电机状态 |
| X-TRACK 的 `Pages/Dialplate`、`LiveMap` | — | 依赖 DataCenter / GPS / 地图库 |
| `Resources/Font/*.c` | 5 | v8 位图字体格式，v9 不兼容 → 改用 Tiny TTF |
| `Resources/Image/*.c` | 41 | 仅转换当前用到的 5 张 |
| `src/` 顶层的 `main.cpp`、`hal/`、`port/`、`web/`、`config.h`、`README.md`、`secrets.h.example` | — | ESP32 专属 / 与 UI 无关 |

同时**删除了本工程原有的 `xknob_assets.{cpp,h}`**——X-Knob 自带 `ResourcePool`，不需要额外那套资源辅助。

### 后续移植 X-TRACK 整套时还缺的模块

`DataCenter`(+`Common/DataProc` 13 个节点)、`Common/HAL` + `USER/HAL`（**X-TRACK 自带 PC 模拟器假 HAL**，见 `Simulator/LVGL.Simulator/HAL/`：GPX 回放当 GPS、`rand()` 当 IMU/MAG、系统时间当 RTC）、`StorageService`、`Filters`、`MapConv`/`TileConv`/`TrackFilter`、自定义控件 `lv_anim_label`/`lv_poly_line`/`lv_img_png`。

---

## 9. 当前状态与已知限制

**已完成**：X-TRACK 版 PageManager（含 `Replace`/`SetRootDefaultStyle`/`onViewUnload`）+ ResourceManager + Menu（旋钮主界面）+ `_Template`，**已编译、链接并运行通过**。
**入口**：`display_xknob()` —— 已接入 `main/src/main.c`（第 111 行调用，原 `test_ui()` 已注释）。分辨率 240×240。

| 限制 | 说明 |
|---|---|
| 二级页全部不可达 | 已安装页面只有 `Template` 与 `Menu`。Menu 的 5 个图标中，4 个分别 Push 到 `SurfaceDial`/`Playground`/`Hass`/`Setting`（均未安装），第 5 个 `System` 原本是关机、现仅打印日志；而 `Template` 虽已安装却**没有入口指向它**。此外 Menu 的页面 root 自身也绑了一个 Push 到 `Playground`（上游如此），故点击空白处同样无反应 |
| 输入以旋钮为主 | 鼠标滚轮 = 旋钮旋转，鼠标中键 = 按下（工程另有鼠标指针与键盘 indev，鼠标左键点击图标也会触发其事件）。这是上游设计——真机是无触摸的圆形旋钮屏 |
| 不支持拖拽滑动 | `PM_State.cpp` 清掉了页面 root 的 `LV_OBJ_FLAG_SCROLLABLE`，`MenuView.cpp` 也清掉了每个 item 的，指针拖拽因此找不到可滚动对象。交互模型是「焦点驱动滚动」 |
| 无实时数值 | 上游旋钮数值由消息框架驱动，砍掉电机后恒为 0 |

---

## 10. 移植约定

后续继续移植（StartUp / Setting / Playground / X-TRACK 各页）时沿用：

1. 移植工作**只允许修改 `main/xknob/CMakeLists.txt`**；根 `CMakeLists.txt` 的 xknob 接入（§6.2）与 `usr_ui/CMakeLists.txt` 均不由移植工作改动。
2. 移植文件里**停用的代码一律注释保留**，标注 `// 原: ...` 与 `// [移植改动] <原因>`，不删除。
3. 代码按「去掉一层 `app/`」的规则放到 `main/xknob/` 下，include 前缀相应为 `Utils/...`、`Resources/...`、`Configs/...`。
4. 对外入口统一为 `display_xknob()`。
5. **移入 `Utils/PageManager/` 的代码请以 X-TRACK 版为准**；成员访问用下划线前缀（`_root`/`_Manager`/`_Name`）。
