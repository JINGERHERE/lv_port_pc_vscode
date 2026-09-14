# xknob —— X-Knob 旋钮 UI 移植

把开源项目 **X-Knob** 的 LVGL 旋钮界面移植到本模拟器工程（LVGL v9.5 / CMake / SDL2 / 无 FreeRTOS）。

---

## 1. 移植来源

| 项 | 值 |
|---|---|
| 源工程 | X-Knob — <https://github.com/SmallPond/X-Knob> |
| 本地路径 | `~/Github/References/X-Knob/1.Firmware` |
| 源代码根 | `1.Firmware/src/`（下文表格中的「来源」均相对此路径） |
| 源 LVGL 版本 | **v8.1**（ESP32-S3 + Arduino + PlatformIO） |
| 目标 LVGL 版本 | **v9.5**（本工程根目录 `lvgl/`） |
| 字体原素材 | `~/Github/References/X-TRACK/ArtDesign/bahnschrift.ttf`、`AGENCYB.TTF` |
| 移植范围 | **仅 UI**。放弃：电机 / 力反馈 / WiFi / MQTT / BLE / 电源管理 / SD 卡 / 消息框架 |

---

## 2. 关于架构：X-Knob 是 X-TRACK 框架的一层

参考项目之间的血缘关系（这也是本目录结构设计的依据）：

```
X-TRACK（LVGL v8，AT32 自行车码表）
   └── 页面框架 PageManager + 消息框架 DataCenter + 各业务页面
        │
        ├──► X-Knob：直接拿来 PageManager，换上「旋钮屏」的 UI 与 AccountSystem
        │            ← 本目录移植的就是这一支
        │
        └──► Peak：同一框架移植到 ESP32（机器人示教盒）
```

换句话说，**X-Knob 的这套 PageManager 框架 = X-TRACK 框架去掉业务层、换成旋钮 UI 后的产物**。X-TRACK 在框架之上还多一层「初始 UI / 业务页面」，而 X-Knob 只保留了框架 + Menu/Setting 等页面。

因此本工程把 **`main/xknob/` 本身当作框架根**（而不是再套一层 `port/app/`）——它的定位就是「一个可复用的页面框架 + 一套旋钮 UI」，将来要接 X-TRACK 的其余页面（表盘、系统信息等）时，可以直接往这个根里加。

---

## 3. 目录结构

```
main/xknob/
├── CMakeLists.txt            独立静态库目标 xknob
├── README.md                 本文档
├── display_xknob.h/.cpp      UI 对外入口 display_xknob()（本工程自建）
├── app.h  app.cpp            装配层：App_Init() 安装页面并进入 Menu
├── Configs/Version.h         版本宏（Menu::Update() 使用）
├── Utils/
│   ├── PageManager/          页面框架（11 文件）
│   └── lv_ext/               LVGL 扩展（6 文件）
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

**include 前缀规则**：以 `main/xknob/` 为 include 根，所以上游 `src/app/` 下的相对路径**原样保留**（只去掉 `app/` 一层）：

| 上游写法 | 移植后 |
|---|---|
| `#include "app/Utils/PageManager/PageManager.h"` | `#include "Utils/PageManager/PageManager.h"` |
| `#include "app/Resources/ResourcePool.h"` | `#include "Resources/ResourcePool.h"` |
| `#include "app/Configs/Version.h"` | `#include "Configs/Version.h"` |
| `#include "app/app.h"` | `#include "app.h"` |
| `#include "../Page.h"` | 不变（目录层级未变） |

---

## 4. 逐文件移植清单

来源 = `X-Knob/1.Firmware/src/app/` 下的路径；目标 = `main/xknob/` 下的**同名路径**（即去掉 `app/` 一层）。
「改动」列：**原样**（逐字节相同）/ **小改** / **重写**。

### 4.1 页面框架 `Utils/PageManager/`（11 文件）

| 来源 | 改动 | 说明 |
|---|---|---|
| `Utils/PageManager/PageManager.h` | 原样 | 页面栈对外接口 |
| `Utils/PageManager/PageBase.h` | 小改 | 补 `<string.h>`——**本文件夹内统一由它提供** |
| `Utils/PageManager/PageFactory.h` | 原样 | 抽象工厂 |
| `Utils/PageManager/PM_Base.cpp` | 原样 | `memset`/`strcmp` 由 `PageManager.h → PageBase.h` 传递 |
| `Utils/PageManager/PM_Anim.cpp` | 原样 | 转场动画参数表 |
| `Utils/PageManager/PM_Router.cpp` | 小改 | `lv_mem_alloc` → `lv_malloc`；另带 `<string.h>`（与 PageBase.h 重复） |
| `Utils/PageManager/PM_State.cpp` | 小改 | `obj->user_data =` → `lv_obj_set_user_data()`；`lv_mem_free` → `lv_free` |
| `Utils/PageManager/PM_Drag.cpp` | 小改 | `lv_event_get_target` → `lv_event_get_target_obj`；`lv_event_send` → `lv_obj_send_event` |
| `Utils/PageManager/PM_Log.h` | 原样 | PC 不定义 `ARDUINO`，自动走 printf 分支 |
| `Utils/PageManager/ResourceManager.h/.cpp` | 原样 | 通用 name→ptr 资源池 |

### 4.2 LVGL 扩展 `Utils/lv_ext/`（6 文件）

| 来源 | 改动 | 说明 |
|---|---|---|
| `Utils/lv_ext/lv_obj_ext_func.h` | 原样 | 含 `lv_get_indev()` 等扩展 |
| `Utils/lv_ext/lv_obj_ext_func.cpp` | 小改 | `indev->driver->type` → `lv_indev_get_type()`；自带 `<string.h>`（够不到 PageBase.h） |
| `Utils/lv_ext/lv_anim_timeline_wrapper.h/.c` | 原样 | 时间轴包装宏 |
| `Utils/lv_ext/lv_label_anim_effect.h/.cpp` | 原样 | 数字滚动切换动画。上游全仓无引用，按要求一并搬入备用 |

### 4.3 资源 `Resources/`

| 来源 | 改动 | 说明 |
|---|---|---|
| `Resources/ResourcePool.h` | 重写 | 去掉 `Image_` 成员；`GetImage()` 改为声明 |
| `Resources/ResourcePool.cpp` | 重写 | 编译期注册 → 运行时加载，见 §5.3 |

### 4.4 页面 `Pages/`

| 来源 | 改动 | 说明 |
|---|---|---|
| `Pages/Page.h` | 小改 | 注释掉 `StatusBar.h` 的 include（StatusBar 未移植） |
| `Pages/AppFactory.h` | 小改 | 补 `#pragma once`（原头文件无 include guard） |
| `Pages/AppFactory.cpp` | 小改 | 注释掉 6 个未移植页面的 include 及其中 5 个的 `APP_CLASS_MATCH`；补 `<string.h>` |
| `Pages/Menu/Menu.h` | 原样 | — |
| `Pages/Menu/Menu.cpp` | 小改 | 见 §5.1、§5.2 |
| `Pages/Menu/MenuView.h` | 小改 | 保持 `#include "../Page.h"`，加说明注释 |
| `Pages/Menu/MenuView.cpp` | 小改 | 修正上游一处字符串拼接笔误，见 §5.3 |
| `Pages/Menu/MenuModel.h/.cpp` | 重写 | 退化为空壳，见 §5.3 |
| `Pages/_Template/Template.h` | 原样 | — |
| `Pages/_Template/Template.cpp` | 小改 | 去 `<Arduino.h>`；`timer->user_data`、`lv_event_get_target` 改写 |
| `Pages/_Template/TemplateView.h` | 原样 | — |
| `Pages/_Template/TemplateView.cpp` | 小改 | `montserrat_10` → `montserrat_14`；占位图 `macos` → `esp_logo` |
| `Pages/_Template/TemplateModel.h/.cpp` | 原样 | — |

### 4.5 装配层（本工程自建 / 重写）

| 目标 | 改动 | 说明 |
|---|---|---|
| `app.h` | 重写 | 只保留三个模式枚举 + `offsetof`/`container_of` 宏，见 §5.2 |
| `app.cpp` | 重写 | `App_Init()` 精简为「Resource.Init + Install Template/Menu + Push Menu」 |
| `display_xknob.h/.cpp` | **新增** | 本工程的 UI 入口，调用 `App_Init()`（对应上游 `src/main.cpp` 的入口角色） |
| `Configs/Version.h` | 原样 | `Menu::Update()` 依赖其 `VERSION_*` 宏 |

**统计**：39 个源文件（18 原样 / 19 有改动 / 2 本工程自建）。

---

## 5. 改动说明

### 5.1 LVGL v8 → v9.5 API 改写（原写法一律以注释保留）

| 原写法（v8） | 现写法（v9.5） | 涉及文件 |
|---|---|---|
| `lv_mem_alloc` / `lv_mem_free` | `lv_malloc` / `lv_free` | `PM_Router.cpp`、`PM_State.cpp` |
| `obj->user_data = x` | `lv_obj_set_user_data(obj, x)` | `PM_State.cpp` |
| `lv_event_get_target()` | `lv_event_get_target_obj()` | `PM_Drag.cpp`、`Menu.cpp`、`Template.cpp` |
| `lv_event_send(obj, ...)` | `lv_obj_send_event(obj, ...)` | `PM_Drag.cpp` |
| `indev->driver->type` | `lv_indev_get_type(indev)` | `lv_obj_ext_func.cpp` |
| `timer->user_data` | `lv_timer_get_user_data(timer)` | `Menu.cpp`、`Template.cpp` |
| `lv_disp_set_bg_color(...)` | `lv_obj_set_style_bg_color(lv_screen_active(), ...)` | `ResourcePool.cpp` |

> v9.5 的 `lvgl.h` 会默认包含 `lv_api_map_v8.h`（仅当定义了 `LV_DISABLE_API_MAPPING` 时才不包含；本工程未定义），因此 `lv_obj_del`、`lv_scr_act`、`lv_group_del`、`lv_img_set_src` 等大量 v8 名称**无需修改**即可编译。上表是兼容层仍未覆盖、必须手改的部分。

### 5.2 剥离 Arduino / ESP32 平台依赖（注释保留，不删除）

| 停用项 | 出现位置 |
|---|---|
| `#include <Arduino.h>` | `MenuModel.cpp`、`Template.cpp` |
| `#include "hal/motor.h"`、`HAL::power_off()` | `Menu.cpp`、`app.h` |
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
| `app.h` | 删掉 AccountSystem 宏、Arduino 任务通知、`display_init` 声明 |
| `app.cpp` | 装配收敛为 `Resource.Init()` → `Install("Template"/"Menu")` → `Push("Pages/Menu")` |
| `Pages/Menu/MenuView.cpp` | **修正上游笔误**：`"Surface Dial" "Control\n"` 两行相邻字面量被 C++ 拼接成 `"Surface DialControl\n"`，已补上 `\n`（原写法以注释保留） |

> **另一个上游自带缺陷**：上游 `app.h` 的 `offsetof` 宏跨行却漏了行尾反斜杠 `\`。ESP32 上 `<Arduino.h>` 已提供标准 `offsetof`，因此该 `#ifndef` 分支从不展开；PC 端去掉 Arduino 依赖后会去展开它，故本次补了 `<stddef.h>` 并修正了该宏。
>
> **`PM_LOG_INFO` 在 PC 上是空操作**：`PM_Log.h` 的 `#else`（非 ARDUINO）分支把 `PM_LOG_INFO` 定义成 `//printf(...)`，而注释在预处理第 3 阶段就被剥离，替换列表因此是**空的**。`PM_LOG_WARN`/`PM_LOG_ERROR` 才是真正的 `printf`。

---

## 6. 构建配置

### 6.1 `main/xknob/CMakeLists.txt`

```cmake
# 递归收集所有子目录下的源码
file(GLOB_RECURSE USR_SOURCES CONFIGURE_DEPENDS *.c *.cpp)
add_library(xknob STATIC ${USR_SOURCES})

# 资源目录：编译期注入绝对路径，ResourcePool 运行时按 "A:<路径>" 加载
target_compile_definitions(xknob PRIVATE
    USR_ASSETS_PREFIX="${CMAKE_CURRENT_SOURCE_DIR}/Resources/"
)

# 本目录即 include 根。
# PUBLIC 是必需的：main/src/main.c 通过 #include "display_xknob.h" 接入本库
target_include_directories(xknob PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(xknob PUBLIC lvgl)
```

### 6.2 根 `CMakeLists.txt`（接入 xknob 目标）

```cmake
add_subdirectory(${PROJECT_SOURCE_DIR}/main/xknob)  # 添加 xknob 子目录

target_link_libraries(main lvgl ... lv_demos_ext
                        usr_ui xknob)               # 追加 xknob
```

### 6.3 未改动

`main/usr_ui/CMakeLists.txt` 保持原样（`usr_ui` 库不受本次移植影响）。

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

| 输出 | 来源 `.c` | 尺寸 | 使用者 |
|---|---|---|---|
| `dialpad.png` | `Resources/Image/img_src_dialpad.c` | 42×42 | Menu |
| `switches.png` | `Resources/Image/img_src_switches.c` | 42×42 | Menu |
| `home.png` | `Resources/Image/img_src_home.c` | 42×42 | Menu |
| `system_info.png` | `Resources/Image/img_src_system_info.c` | 36×36 | Menu |
| `setting.png` | `Resources/Image/img_src_setting.c` | 42×42 | Menu |
| `esp_logo.png` | 本工程原有 | 96×96 | `_Template` 占位 |

转换命令（脚本解析 `.c` 中 `#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP == 0` 分支，按「RGB565 小端 2 字节 + A8 1 字节」逐像素还原为 RGBA PNG，仅用 Python 标准库）：

```bash
python3 main/xknob/tools/v8img2png.py <in.c> <out.png> [...]
```

> **另有 20 张中文名 PNG**（`指南针.png`、`电池信息.png`、`菜单.png` …），是从 X-TRACK `ArtDesign/` 拷来的素材，**文件名与代码请求的英文资源名不一致**（代码要 `compass`、`battery_info`、`menu`）。当前 Menu/Template 用不到它们；将来启用对应页面时需要改名或加一层映射。

---

## 8. 未移植内容（范围声明）

| 上游路径（`src/app/` 下） | 文件数 | 未移植原因 |
|---|---|---|
| `Utils/AccountSystem/` | 7 | 消息框架；数据生产者全是被放弃的 HAL |
| `Accounts/` | 6 | 同上 |
| `Pages/StatusBar/` | 2 | 死代码（调用点全被注释）且依赖 Account/HAL |
| `Pages/SystemInfos/` | 6 | 机器人关节/IMU，无引用 |
| `Pages/StartUp/` | 6 | 开机动画页，Phase 2 计划 |
| `Pages/Playground/` | 6 | 依赖电机；含 `lv_meter`，需改 `lv_scale` |
| `Pages/Setting/` | 6 | 依赖 WiFi/NVS；含 `lv_meter` |
| `Pages/HASS/` | 6 | 依赖 MQTT |
| `Pages/SurfaceDial/` | 6 | 依赖 BLE + 电机状态 |
| `Resources/Font/*.c` | 5 | v8 位图字体格式，v9 不兼容 → 改用 Tiny TTF |
| `Resources/Image/*.c` | 41 | 仅转换当前用到的 5 张 |
| `src/` 顶层的 `main.cpp`、`hal/`、`port/`、`web/`、`config.h`、`README.md`、`secrets.h.example` | — | ESP32 专属 / 与 UI 无关 |

同时**删除了本工程原有的 `xknob_assets.{cpp,h}`**——X-Knob 自带 `ResourcePool`，不需要额外那套资源辅助。

---

## 9. 当前状态与已知限制

**已完成**：PageManager 框架 + Menu（旋钮主界面）+ `_Template`，**已编译并链接通过**。
**入口**：`display_xknob()` —— 已接入 `main/src/main.c`（第 111 行调用，原 `test_ui()` 已注释）。分辨率为 240×240。

| 限制 | 说明 |
|---|---|
| 二级页全部不可达 | 已安装页面只有 `Template` 与 `Menu`。Menu 的 5 个图标中，4 个分别 Push 到 `SurfaceDial`/`Playground`/`Hass`/`Setting`（均未安装），第 5 个 `System` 原本是关机、现仅打印日志；而 `Template` 虽已安装却**没有入口指向它**。此外 Menu 的页面 root 自身也绑了一个 Push 到 `Playground`（上游如此），故点击空白处同样无反应 |
| 输入以旋钮为主 | 鼠标滚轮 = 旋钮旋转，鼠标中键 = 按下（工程另有鼠标指针与键盘 indev，鼠标左键点击图标也会触发其事件）。这是上游设计——真机是无触摸的圆形旋钮屏 |
| 不支持拖拽滑动 | `PM_State.cpp` 清掉了页面 root 的 `LV_OBJ_FLAG_SCROLLABLE`，`MenuView.cpp` 也清掉了每个 item 的，指针拖拽因此找不到可滚动对象。交互模型是「焦点驱动滚动」 |
| 无实时数值 | 上游旋钮数值由 `MotorStatus` 消息驱动，砍掉电机后恒为 0 |

---

## 10. 移植约定

后续继续移植（StartUp / Setting / Playground 等）时沿用：

1. 只修改 `main/xknob/CMakeLists.txt`；根 `CMakeLists.txt` 的 xknob 接入（§6.2）不由移植工作改动。
2. 移植文件里**停用的代码一律注释保留**，标注 `// 原: ...` 与 `// [移植改动] <原因>`，不删除。
3. 代码按「去掉 `app/` 一层」的规则放到 `main/xknob/` 下，include 前缀相应为 `Utils/...`、`Resources/...`、`Configs/...`。
4. 对外入口统一为 `display_xknob()`。
