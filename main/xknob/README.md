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

> X-Knob 的页面框架（PageManager）继承自 [X-TRACK](https://github.com/FASTSHIFT/X-TRACK)，本工程后续再接 X-TRACK 的其余页面时可复用同一套框架。

---

## 2. 目录结构

```
main/xknob/
├── CMakeLists.txt          独立静态库目标 xknob
├── display_xknob.cpp       UI 对外入口 display_xknob() —— 调用 App_Init()
├── xknob_assets.cpp        资源路径辅助（本工程原有，未改动）
├── include/                对外头文件（原有）
├── priv_include/  private/ 预留
├── assets/                 运行时资源：字体 TTF + 图标 PNG
├── tools/
│   └── v8img2png.py        一次性转换脚本：LVGL v8 图片 .c 数组 → PNG
└── port/                   【移植代码】镜像上游 src/ 的 app/ 树
    └── app/
        ├── app.h  app.cpp              装配层
        ├── Configs/Version.h
        ├── Utils/PageManager/          页面框架（11 文件）
        ├── Utils/lv_ext/               LVGL 扩展（6 文件）
        ├── Resources/ResourcePool.*    资源池
        └── Pages/
            ├── Page.h  AppFactory.*    页面公共头 + 工厂
            ├── Menu/                   旋钮主界面
            └── _Template/              空白模板页
```

### 为什么是 `port/app/` 这一层

上游的 include 习惯是 `#include "app/Utils/..."`、`#include "../Page.h"`。把 `port/` 设为 include 根并**保持上游目录层级不变**，这些路径就全部原样生效——这是本次移植几乎不改 include 路径的原因。对应改动在 `CMakeLists.txt`（见 §5）。

---

## 3. 逐文件移植清单

来源 = `X-Knob/1.Firmware/src/` 下的路径；目标 = `main/xknob/` 下的路径。
「改动」列：**原样**（逐字节相同）/ **小改** / **重写**。

### 3.1 页面框架 `Utils/PageManager/`（11 文件）

| 来源 | 目标 | 改动 | 说明 |
|---|---|---|---|
| `app/Utils/PageManager/PageManager.h` | `port/app/Utils/PageManager/PageManager.h` | 原样 | 页面栈对外接口 |
| `app/Utils/PageManager/PageBase.h` | 同上目录 | 小改 | 补 `<string.h>`——本文件夹内**统一由它提供**（原本靠 Arduino.h 传递 `memcpy`） |
| `app/Utils/PageManager/PageFactory.h` | 同上 | 原样 | 抽象工厂 |
| `app/Utils/PageManager/PM_Base.cpp` | 同上 | 原样 | 用到的 `memset`/`strcmp` 由 `PageManager.h → PageBase.h` 传递 |
| `app/Utils/PageManager/PM_Anim.cpp` | 同上 | 原样 | 同上（`memset`） |
| `app/Utils/PageManager/PM_Router.cpp` | 同上 | 小改 | `lv_mem_alloc` → `lv_malloc`；（另加的 `<string.h>` 与 PageBase.h 重复，属冗余） |
| `app/Utils/PageManager/PM_State.cpp` | 同上 | 小改 | `root_obj->user_data =` → `lv_obj_set_user_data()`；`lv_mem_free` → `lv_free` |
| `app/Utils/PageManager/PM_Drag.cpp` | 同上 | 小改 | `lv_event_get_target` → `lv_event_get_target_obj`；`lv_event_send` → `lv_obj_send_event` |
| `app/Utils/PageManager/PM_Log.h` | 同上 | 原样 | PC 不定义 `ARDUINO`，自动走 printf 分支 |
| `app/Utils/PageManager/ResourceManager.h` | 同上 | 原样 | 通用 name→ptr 资源池 |
| `app/Utils/PageManager/ResourceManager.cpp` | 同上 | 原样 | — |

> **关于 `<string.h>`**：上游这些文件在 ESP32 上靠 `<Arduino.h>` 间接获得 `memcpy`/`memset`/`strcmp`。
> 实测 `lvgl.h` 的包含闭包（330 个文件）**不含** `<string.h>`/`<stdio.h>`/`<stdlib.h>`，故必须自行引入。
> 本文件夹采用「集中提供」：`PageManager.h` 一开头就 include `PageBase.h`，因此凡经 `PageManager.h` 进入的文件
> （`PM_Anim` / `PM_Base` / `PM_Drag` / `PM_Router` / `PM_State`）都能拿到它，无需各自重复 include。
>
> 但有两个文件**够不到** `PageBase.h`，必须自带：`ResourceManager.cpp`（只 include `ResourceManager.h` + `PM_Log.h`）、
> `lv_obj_ext_func.cpp`（只 include `lv_obj_ext_func.h`）。

### 3.2 LVGL 扩展 `Utils/lv_ext/`（6 文件）

| 来源 | 目标 | 改动 | 说明 |
|---|---|---|---|
| `app/Utils/lv_ext/lv_obj_ext_func.h` | `port/app/Utils/lv_ext/` | 原样 | 含 `lv_get_indev()` 等扩展 |
| `app/Utils/lv_ext/lv_obj_ext_func.cpp` | 同上 | 小改 | `cur_indev->driver->type` → `lv_indev_get_type()`；补 `<string.h>` |
| `app/Utils/lv_ext/lv_anim_timeline_wrapper.h` | 同上 | 原样 | 时间轴包装宏 |
| `app/Utils/lv_ext/lv_anim_timeline_wrapper.c` | 同上 | 原样 | 其用到的 API 在 v9.5 均存在 |
| `app/Utils/lv_ext/lv_label_anim_effect.{h,cpp}` | 同上 | 原样 | 数字滚动切换标签效果。上游全仓无引用，**按用户要求一并搬入**备用（当前无调用点） |

### 3.3 资源池 `Resources/`

| 来源 | 目标 | 改动 | 说明 |
|---|---|---|---|
| `app/Resources/ResourcePool.h` | `port/app/Resources/` | 重写 | 去掉 `Image_` 成员；`GetImage()` 改为声明（实现移到 .cpp） |
| `app/Resources/ResourcePool.cpp` | 同上 | 重写 | 见 §4.3 |

### 3.4 页面 `Pages/`

| 来源 | 目标 | 改动 | 说明 |
|---|---|---|---|
| `app/Pages/Page.h` | `port/app/Pages/` | 小改 | 注释掉 `StatusBar.h` 的 include（StatusBar 未移植） |
| `app/Pages/AppFactory.h` | 同上 | 小改 | 补 `#pragma once`（原头文件无 include guard） |
| `app/Pages/AppFactory.cpp` | 同上 | 小改 | 注释掉 6 个未移植页面的 include，以及其中 5 个页面的 `APP_CLASS_MATCH`（`SystemInfos` 上游本就没有该条目）；补 `<string.h>` |
| `app/Pages/Menu/Menu.h` | `port/app/Pages/Menu/` | 原样 | — |
| `app/Pages/Menu/Menu.cpp` | 同上 | 小改 | 见 §4.1 与 §4.2 | 
| `app/Pages/Menu/MenuView.h` | 同上 | 小改 | 保持 `#include "../Page.h"`，加说明注释 |
| `app/Pages/Menu/MenuView.cpp` | 同上 | 小改 | 加说明注释（本身无需改） |
| `app/Pages/Menu/MenuModel.h` | 同上 | **重写** | 退化为空壳，见 §4.3 |
| `app/Pages/Menu/MenuModel.cpp` | 同上 | **重写** | 同上；原实现整段注释保留 |
| `app/Pages/_Template/Template.h` | `port/app/Pages/_Template/` | 原样 | — |
| `app/Pages/_Template/Template.cpp` | 同上 | 小改 | 去 `<Arduino.h>`；`timer->user_data` 与 `lv_event_get_target` 改写 |
| `app/Pages/_Template/TemplateView.h` | 同上 | 原样 | — |
| `app/Pages/_Template/TemplateView.cpp` | 同上 | 小改 | `montserrat_10` → `montserrat_14`；占位图 `macos` → `esp_logo` |
| `app/Pages/_Template/TemplateModel.h` | 同上 | 原样 | — |
| `app/Pages/_Template/TemplateModel.cpp` | 同上 | 原样 | — |

### 3.5 装配层与配置

| 来源 | 目标 | 改动 | 说明 |
|---|---|---|---|
| `app/app.h` | `port/app/` | **重写** | 只保留三个模式枚举 + `offsetof`/`container_of` 宏；见 §4.3 |
| `app/app.cpp` | 同上 | **重写** | `App_Init()` 精简为「Resource.Init + Install Template/Menu + Push Menu」 |
| `app/Configs/Version.h` | `port/app/Configs/` | 原样 | `Menu::Update()` 依赖其 `VERSION_*` 宏 |

---

## 4. 改动说明

### 4.1 LVGL v8 → v9.5 API 改写（全部以注释保留原写法）

| 原写法（v8） | 现写法（v9.5） | 涉及文件 |
|---|---|---|
| `lv_mem_alloc` / `lv_mem_free` | `lv_malloc` / `lv_free` | `PM_Router.cpp`、`PM_State.cpp` |
| `obj->user_data = x` | `lv_obj_set_user_data(obj, x)` | `PM_State.cpp` |
| `lv_event_get_target()` | `lv_event_get_target_obj()` | `PM_Drag.cpp`、`Menu.cpp`、`Template.cpp` |
| `lv_event_send(obj, ...)` | `lv_obj_send_event(obj, ...)` | `PM_Drag.cpp` |
| `indev->driver->type` | `lv_indev_get_type(indev)` | `lv_obj_ext_func.cpp` |
| `timer->user_data` | `lv_timer_get_user_data(timer)` | `Menu.cpp`、`Template.cpp` |
| `lv_disp_set_bg_color(...)` | `lv_obj_set_style_bg_color(lv_screen_active(), ...)` | `ResourcePool.cpp` |

> v9.5 的 `lvgl.h` 会默认包含 `lv_api_map_v8.h`（仅当定义了 `LV_DISABLE_API_MAPPING` 时才不包含；本工程未定义该宏），因此 `lv_obj_del`、`lv_scr_act`、`lv_group_del`、`lv_img_set_src` 等大量 v8 名称**无需修改**即可编译；上表是兼容层仍未覆盖、必须手改的部分。

### 4.2 剥离 Arduino / ESP32 平台依赖（注释保留，不删除）

| 停用项 | 出现位置 |
|---|---|
| `#include <Arduino.h>` | `MenuModel.cpp`、`Template.cpp` |
| `#include "hal/motor.h"`、`HAL::power_off()` | `Menu.cpp`、`app.h` |
| `#include "hal/hal.h"`、`Accounts_Init()` | `app.cpp` |
| `#include "app/Accounts/Account_Master.h"` | `MenuModel.h`、`app.cpp` |
| `#include "app/Pages/StatusBar/StatusBar.h"` | `Page.h`、`app.cpp` |
| `Serial.printf` | 改为标准 `printf` |
| `ACCOUNT_SEND_NOTIFY_CMD`、`display_init()` | `app.h` |

### 4.3 重写与新增

| 文件 | 处理 |
|---|---|
| `port/app/Pages/Menu/MenuModel.{h,cpp}` | **退化为空壳**。原 `Init/Deinit/ChangeMotorMode` 依赖 AccountSystem 与电机 HAL，整段注释保留；本页 UI 并不消费电机数据 |
| `port/app/Resources/ResourcePool.cpp` | 由「编译期注册位图字体/图片描述符」改为「运行时加载」：字体走 Tiny TTF，图片走 `"A:"` 路径 + lodepng |
| `port/app/app.h` | 删掉 AccountSystem 宏、Arduino 任务通知、`display_init` 声明；补 `#include <stddef.h>`（见下方注意事项） |
| `port/app/app.cpp` | 装配收敛为 `Resource.Init()` → `Install("Template"/"Menu")` → `Push("Pages/Menu")`；其余页面安装语句注释保留 |
| `display_xknob.cpp` | 入口改为调用 `App_Init()` |
| `tools/v8img2png.py` | **新增**。把 v8 图片 `.c` 数组还原为 PNG |
| `assets/*.png`、`assets/*.ttf` | **新增**（脚本生成 / 从 X-TRACK 拷贝） |

> **注意（上游自带缺陷）**：上游 `app.h` 的 `offsetof` 宏跨行却漏了行尾反斜杠 `\`。ESP32 上 `<Arduino.h>` 已提供标准 `offsetof`，因此该 `#ifndef` 分支从不展开；PC 端去掉 Arduino 依赖后会去展开它，故本次补了 `<stddef.h>` 并修正了该宏。

---

## 5. 构建配置改动

涉及两个文件：

### 5.1 `main/xknob/CMakeLists.txt`（库目标内部，2 处）

```cmake
# 1) GLOB → GLOB_RECURSE，递归收集 port/ 下的源码
file(GLOB_RECURSE USR_SOURCES CONFIGURE_DEPENDS *.c *.cpp)

# 2) 把 port/ 加为 include 根，使 "app/..." / "hal/..." 前缀可解析
target_include_directories(xknob
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/priv_include
            ${CMAKE_CURRENT_SOURCE_DIR}/port
)
```

### 5.2 根 `CMakeLists.txt`（接入 xknob 目标，2 处）

```cmake
add_subdirectory(${PROJECT_SOURCE_DIR}/main/usr_ui) # 添加 usr_ui 子目录
add_subdirectory(${PROJECT_SOURCE_DIR}/main/xknob)  # 添加 xknob 子目录   ← 新增

target_link_libraries(main lvgl ... lv_demos_ext
                        usr_ui xknob)               # ← 追加 xknob
```

### 未改动

`main/usr_ui/CMakeLists.txt` 保持原样（`usr_ui` 库不受本次移植影响）。

---

## 6. 资源

### 字体（Tiny TTF，运行时加载，无需 lv_font_conv）

| 文件 | 来源 | 用途 |
|---|---|---|
| `assets/bahnschrift.ttf` | X-TRACK `ArtDesign/` | `bahnschrift_13`、`bahnschrift_17`（Menu 在用） |
| `assets/AGENCYB.TTF` | X-TRACK `ArtDesign/` | `agencyb_36`（暂未启用） |

依赖配置：`lv_conf.h` 中 `LV_USE_TINY_TTF=1`、`LV_TINY_TTF_FILE_SUPPORT=1`、`LV_USE_FS_STDIO=1`（盘符 `'A'`）。
在 `ResourcePool::Init()` 中创建并常驻（Tiny TTF 是堆对象，不可反复创建）。

### 图片（v8 `.c` → PNG）

| 输出 | 来源 `.c` | 尺寸 |
|---|---|---|
| `dialpad.png` | `app/Resources/Image/img_src_dialpad.c` | 42×42 |
| `switches.png` | `app/Resources/Image/img_src_switches.c` | 42×42 |
| `home.png` | `app/Resources/Image/img_src_home.c` | 42×42 |
| `system_info.png` | `app/Resources/Image/img_src_system_info.c` | 36×36 |
| `setting.png` | `app/Resources/Image/img_src_setting.c` | 42×42 |

转换命令：

```bash
python3 main/xknob/tools/v8img2png.py <in.c> <out.png> [...]
```

脚本解析 `.c` 中 `#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP == 0` 分支，按「RGB565 小端 2 字节 + A8 1 字节」逐像素还原为 RGBA PNG（仅用 Python 标准库）。

---

## 7. 未移植内容（范围声明）

| 上游路径 | 文件数 | 未移植原因 |
|---|---|---|
| `app/Utils/AccountSystem/` | 7 | 消息框架；其数据生产者全是被放弃的 HAL |
| `app/Accounts/` | 6 | 同上 |
| `app/Pages/StatusBar/` | 2 | 死代码（调用点全被注释）且依赖 Account/HAL |
| `app/Pages/SystemInfos/` | 6 | 机器人关节/IMU，无引用 |
| `app/Pages/StartUp/` | 6 | 开机动画页，Phase 2 计划 |
| `app/Pages/Playground/` | 6 | 依赖电机；含 `lv_meter`，需改 `lv_scale` |
| `app/Pages/Setting/` | 6 | 依赖 WiFi/NVS；含 `lv_meter` |
| `app/Pages/HASS/` | 6 | 依赖 MQTT |
| `app/Pages/SurfaceDial/` | 6 | 依赖 BLE + 电机状态 |
| `app/Resources/Font/` | 5 | v8 位图字体格式，v9 不兼容 → 改用 Tiny TTF |
| `app/Resources/Image/` | 41 | 仅转换当前用到的 5 张 |
| `src/main.cpp`、`src/hal/`、`src/port/`、`src/web/`、`src/config.h`、`src/README.md`、`src/secrets.h.example` | — | ESP32 专属 / 与 UI 无关 |

---

## 8. 当前状态与已知限制

**已完成**：PageManager 框架 + Menu（旋钮主界面）+ `_Template`，可编译运行。
**入口**：`display_xknob()` —— **已接入** `main/src/main.c`（第 111 行调用，原 `test_ui()` 已注释）。

| 限制 | 说明 |
|---|---|
| 二级页全部不可达 | 已安装页面只有 `Template` 与 `Menu`。Menu 的 5 个图标中，4 个分别 Push 到 `SurfaceDial`/`Playground`/`Hass`/`Setting`（均未安装），第 5 个 `System` 原本是关机、现仅打印日志；而 `Template` 虽已安装却**没有入口指向它**。此外 Menu 的页面 root 自身也绑了一个 Push 到 `Playground`（上游如此），故点击空白处同样无反应 |
| 输入以旋钮为主 | 鼠标滚轮 = 旋钮旋转，鼠标中键 = 按下（工程另有鼠标指针与键盘 indev，鼠标左键点击图标也会触发其事件）。这是上游设计——真机是无触摸的圆形旋钮屏 |
| 不支持拖拽滑动 | `PM_State.cpp` 清掉了页面 root 的 `LV_OBJ_FLAG_SCROLLABLE`，`MenuView.cpp` 也清掉了每个 item 的，指针拖拽因此找不到可滚动对象。交互模型是「焦点驱动滚动」 |
| 无实时数值 | 上游旋钮数值由 `MotorStatus` 消息驱动，砍掉电机后恒为 0 |

---

## 9. 移植约定

后续继续移植（StartUp / Setting / Playground 等）时沿用：

1. 移植工作**只允许修改 `main/xknob/CMakeLists.txt`**；根 `CMakeLists.txt` 的 xknob 接入（§5.2）与 `usr_ui/CMakeLists.txt` 均不由移植工作改动。
2. 移植文件里**停用的代码一律注释保留**，标注 `// 原: ...` 与 `// [移植改动] <原因>`，不删除。
3. 移植代码放在 `port/` 下并**保持上游目录层级**，使 `app/...` 前缀 include 原样生效。
4. 对外入口统一为 `display_xknob()`。
