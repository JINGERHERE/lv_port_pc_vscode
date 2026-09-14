# xknob —— X-TRACK 可复用 UI 框架 + X-Knob 旋钮 UI 移植

把 **X-TRACK** 那套 UI 框架（页面管理 + 数据总线 + 工具库 + 自定义控件）与 **X-Knob** 的旋钮界面，
一起移植到本模拟器工程（LVGL v9.5 / CMake / SDL2 / 无 FreeRTOS）。

> **定位**：本目录提供的是一套**可复用的 UI 框架**——具体页面、菜单、开屏页等布局**交给调用方自己写**。
> 因此移植时严格按「**框架件 / 工具件 / 应用件**」划线，只搬前两类（见 §2.2）。

---

## 1. 移植来源

本目录**有两个来源**——框架层跟踪 X-TRACK，页面层跟踪 X-Knob：

| 来源 | 本地路径 | 提供什么 |
|---|---|---|
| **X-TRACK** | `~/Github/References/X-TRACK/Software/X-Track/USER/App` | `Utils/PageManager/`（页面框架）、`Utils/ResourceManager/`（资源注册表）、`Utils/DataCenter/`（数据总线）、`Utils/Filters/`、`Utils/PointContainer/`、`Utils/Time/`、`Utils/StorageService/`、`Utils/ArduinoJson/`、`Utils/lv_anim_label/` |
| ↑ 同仓库、但**在 `App/` 之外** | `~/Github/References/X-TRACK/Software/X-Track/USER/` | `HAL/`（真机驱动，18 文件）、`lv_port/`（显示/输入/文件系统移植层）、`main.cpp` —— 均属**应用/平台件，不搬** |
| **X-Knob** | `~/Github/References/X-Knob/1.Firmware/src/app` | `Pages/`（Menu、_Template）、`Configs/`、`Resources/`、`Utils/lv_ext/`、`app.h`/`app.cpp` |
| 字体原素材 | `~/Github/References/X-TRACK/ArtDesign/` | `bahnschrift.ttf`、`AGENCYB.TTF` |

| 项 | 值 |
|---|---|
| 源 LVGL 版本 | X-TRACK **v8.3** ／ X-Knob **v8.1**（ESP32-S3 + Arduino + PlatformIO） |
| 目标 LVGL 版本 | **v9.5**（本工程根目录 `lvgl/`） |
| 移植范围 | **仅 UI + 框架**。放弃：电机 / 力反馈 / WiFi / MQTT / BLE / 电源管理 / SD 卡 / 码表业务 |

> **为什么 PageManager 取 X-TRACK 而不是 X-Knob**：两者同源，但 X-Knob 做了裁剪（删掉了 `Replace`、`SetRootDefaultStyle`、`onViewUnload` 钩子）并改了成员命名。X-TRACK 版最完整。

---

## 2. 关于架构

### 2.1 X-Knob 是 X-TRACK 框架的一层

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

**X-Knob 的这套 PageManager 框架 = X-TRACK 框架去掉业务层、换成旋钮 UI 后的产物。** 因此本工程把 **`main/xknob/` 本身当作框架根**。

### 2.2 移植边界：框架件 / 工具件 / 应用件

判定标准（一句话）：

> **把模块里所有领域名词（GPS、里程、瓦片、经纬度、卫星数…）删干净后，还剩不剩一件「活」？**
> 剩下的是**定义控制流 / 契约**的 → **框架件**；是**自成体系的通用算法 / 工具** → **工具件**；删干净就空了、或它本身就是一份「内容清单」→ **应用件**；零引用 → **死代码**。

关键区分：**「被用于某场景」≠「内含该场景语义」**。`Filters` 被码表用来滤速度，但模板里只有 `T value/lastValue`，它仍是通用滤波。

| 归类 | 本目录里有什么 | 换项目时 |
|---|---|---|
| **框架件** | `PageManager`（页面流转）、`DataCenter`（数据流转）、`ResourceManager`/`ResourcePool` 门面、`lv_ext`/`lv_anim_label` 控件、`_Template` 脚手架、`Page.h` 公共头 | 直接用 |
| **工具件** | `Filters`（模板滤波）、`PointContainer`（压缩点容器）、`TileConv`（视口↔瓦片网格纯几何）、`Time`、`StorageService`（+`ArduinoJson`） | 按需取用 |
| **应用件** | X-TRACK 的 `Pages/`（表盘/地图/系统信息…）、`Common/DataProc` 的 12 个 `DP_*.cpp`（`DP_LIST.inc` 定义 13 个节点）、`Common/HAL` 的 GPS/SportStatus 结构体、`USER/HAL/` 真机驱动、`MapConv`/`TrackFilter`/`GPX` | **不搬**，交调用方 |
| **死代码 / 不搬** | `lv_img_png`（依赖 v8 绘制管线）、`new/`（在我们 Release 构建下会生效且有害）、`lv_allocator`、`Stream` | 不搬 |

> **`Common/HAL` 与 `Common/DataProc` 属「一半框架一半应用」**，本次**未搬**：`HAL.h` 共 **14 个接口族**，只有 GPS 族是码表专属，其余 **13 族**（Backlight / Display / FaultHandle / I2C / IMU / MAG / SD / Power / Clock / Buzzer / Encoder / Audio / Memory）通用；`DataProc.cpp` 的 26 行节点管理器是通用机制、**12 个 `DP_*.cpp`** 是应用内容（`DP_LIST.inc` 另有 **13 个节点定义**）。若将来需要「数据节点」这套机制，应拆成「通用机制 + 应用内容」两层再取。

---

## 3. 目录结构

```
main/xknob/
├── CMakeLists.txt            独立静态库目标 xknob
├── README.md                 本文档
├── display_xknob.h/.cpp      UI 对外入口 display_xknob()（本工程自建）
├── app.h  app.cpp            装配层：App_Init() 设根样式、安装页面并进入 Menu
├── Configs/Version.h         版本宏（Menu::Update() 使用）
├── Utils/                    ── 框架件 & 工具件（均来自 X-TRACK，除 lv_ext 外）
│   ├── PageManager/          页面框架：页面栈 / 路由 / 转场 / 生命周期（10 文件）
│   ├── DataCenter/           数据总线：Account 发布订阅 + PingPongBuffer（7 文件）
│   ├── ResourceManager/      资源注册表：名字 → void*（2 文件）
│   ├── StorageService/       JSON 文件驱动的 KV 持久化（2 文件）
│   ├── Filters/              header-only C++ 模板滤波库（7 文件）
│   ├── PointContainer/       超长二维点序列的差分压缩容器（2 文件）
│   ├── Time/                 Arduino TimeLib 移植（20 文件，含 examples/）
│   ├── lv_anim_label/        滚动切换文字控件（2 文件）
│   ├── lv_ext/               LVGL 扩展（6 文件，来自 X-Knob）
│   └── ArduinoJson/          第三方 JSON 库 v6.18.4（426 文件，header-only）
├── Resources/
│   ├── ResourcePool.h/.cpp   资源池：字体 + 图片
│   ├── Font/                 bahnschrift.ttf、AGENCYB.TTF
│   └── Image/                图标 PNG（6 张英文名在用 + 20 张 X-TRACK 素材）
├── Pages/                    ── 页面层（来自 X-Knob；属「示例应用」，可整目录替换）
│   ├── Page.h  AppFactory.h/.cpp
│   ├── Menu/                 旋钮主界面（6 文件）
│   └── _Template/            空白模板页（6 文件）
└── tools/
    └── v8img2png.py          离线工具：LVGL v8 图片 .c 数组 → PNG
```

**文件规模**：全部文件 **542** 个（含 5 个 macOS `.DS_Store`；不计它们则为 **537**）；其中 `.c/.cpp/.h` **242** 个；**参与编译的 `.c/.cpp` 共 28 个**（其余是 ArduinoJson 的头文件与不参与编译的第三方测试/示例，见 §6.1）。

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

| 文件 | 改动 | 说明 |
|---|---|---|
| `PageManager.h` | 原样 | 页面栈对外接口，含 `Replace` / `SetRootDefaultStyle` |
| `PageBase.h` | 小改 | 补 `<string.h>`（集中覆盖本目录 4 个 .cpp 的用量） |
| `PageBase.cpp` | 小改 | `lv_mem_free` → `lv_free`（X-Knob 版把本文件内容内联进了头文件，故这是**新增文件**） |
| `PageFactory.h` | 原样 | 抽象工厂 |
| `PM_Anim.cpp` | 原样 | 转场动画参数表 |
| `PM_Base.cpp` | 原样 | 页面池 / 页面栈 / 装配 API |
| `PM_Router.cpp` | 小改 | `lv_mem_alloc` → `lv_malloc`。本文件含 `Replace`/`Push`/`Pop`/`BackHome`/`SwitchTo` |
| `PM_State.cpp` | 小改 | `lv_mem_free` → `lv_free` |
| `PM_Drag.cpp` | 小改 | `lv_event_get_current_target` → `..._obj`；`lv_event_send` → `lv_obj_send_event`；补 `<algorithm>`/`<cstdlib>` |
| `PM_Log.h` | 原样 | ARDUINO 分支定义成空宏，**完全不依赖 `<Arduino.h>`** |

### 4.2 数据总线 `Utils/DataCenter/`（7 文件，来源 **X-TRACK**）

| 文件 | 改动 | 说明 |
|---|---|---|
| `Account.cpp` | 小改 | 3 处 v9 补丁，见 §5.1 |
| `Account.h` / `DataCenter.h` / `DataCenter.cpp` / `DataCenterLog.h` | 原样 | 发布 / 订阅 / 拉取 / 通知四态总线 |
| `PingPongBuffer/PingPongBuffer.{c,h}` | 原样 | 无锁双缓冲（纯 C，零依赖） |

> 零应用语义（节点名由调用方给）。它是框架的「数据侧另一半」：PageManager 管页面流转，DataCenter 管数据流转。

### 4.3 工具库（来源 **X-TRACK**）

| 模块 | 文件数 | 改动 | 说明 |
|---|---|---|---|
| `Utils/ResourceManager/` | 2 | 原样 | 名字 → `void*` 注册表 + `SetDefault` 兜底（**属「框架件」，见 §2.2**；因与工具件同批搬入而列在本表） |
| `Utils/Filters/` | 7 | 原样 | header-only C++ **模板**滤波（低通 / 中值 / 滑动中值 / 滑动限幅 / 迟滞 —— 7 文件 = `FilterBase.h` + `Filters.h` + 5 个滤波类） |
| `Utils/PointContainer/` | 2 | 原样 | 二维点序列**差分压缩**容器（`{int8 x,y}` 差分 + 超限退化为 `{int32 x,y}`） |
| `Utils/Time/` | 20 | 原样 | Arduino TimeLib 移植（含 `examples/`、`library.json` 等元数据，文件完整） |
| `Utils/StorageService/` | 2 | 小改 | JSON 文件驱动的 KV 持久化；1 处 v9 补丁，见 §5.1 |
| `Utils/ArduinoJson/` | 426 | 原样 | 第三方 JSON 库 v6.18.4，header-only，**文件完整**（含 `extras/`、`examples/`，但不参与编译，见 §6.1） |

> **`StorageService` 的用法要点**：`Add(key, &value, size, type)` 存的是**变量地址指针**（不拷贝值），
> Save/Load 时直接读写那个变量 → **被注册的变量必须活到 StorageService 之后**（通常用全局/静态变量）。
> 它只调 `lv_fs_open/read/write`，**不关心数据落到哪**——换后端（SD 卡 / 片内 Flash / PC 文件）不改一行代码。

### 4.4 自定义控件 `Utils/lv_anim_label/`（2 文件，来源 **X-TRACK**）

| 文件 | 改动 | 说明 |
|---|---|---|
| `lv_anim_label.h` | 小改 | **加 `#include "lvgl_private.h"`**，见 §5.3 |
| `lv_anim_label.c` | 原样 | 双 label 滚动切字控件（`create` + 8 个 setter + `push_text`） |

### 4.5 LVGL 扩展 `Utils/lv_ext/`（6 文件，来源 **X-Knob**）

| 文件 | 改动 | 说明 |
|---|---|---|
| `lv_obj_ext_func.h` | 原样 | 含 `lv_get_indev()` 等扩展 |
| `lv_obj_ext_func.cpp` | 小改 | `indev->driver->type` → `lv_indev_get_type()`；自带 `<string.h>` |
| `lv_anim_timeline_wrapper.h/.c` | 原样 | 时间轴包装宏 |
| `lv_label_anim_effect.h/.cpp` | 原样 | 数字滚动切换动画（上游全仓无引用，按要求一并搬入备用） |

### 4.6 页面 `Pages/`（来源 **X-Knob**）

| 文件 | 改动 | 说明 |
|---|---|---|
| `Page.h` | 小改 | 注释掉 `StatusBar.h` 的 include（StatusBar 未移植） |
| `AppFactory.h` | 小改 | 补 `#pragma once`（原头文件无 include guard） |
| `AppFactory.cpp` | 小改 | 注释掉 6 个未移植页面的 include 及其中 5 个的 `APP_CLASS_MATCH`；补 `<string.h>` |
| `Menu/Menu.h` | 原样 | — |
| `Menu/Menu.cpp` | 小改 | 见 §5.1、§5.2；成员访问改为 `_root`/`_Manager` |
| `Menu/MenuView.h` | 小改 | 保持 `#include "../Page.h"`，加说明注释 |
| `Menu/MenuView.cpp` | 小改 | 修正上游一处字符串拼接笔误，见 §5.3 |
| `Menu/MenuModel.h/.cpp` | 重写 | 退化为空壳，见 §5.3 |
| `_Template/Template.h` | 原样 | — |
| `_Template/Template.cpp` | 小改 | 去 `<Arduino.h>`；`timer->user_data`、`lv_event_get_target` 改写；成员访问改为 `_root`/`_Name`/`_Manager` |
| `_Template/TemplateView.h` | 原样 | — |
| `_Template/TemplateView.cpp` | 小改 | `montserrat_10` → `montserrat_14`；占位图 `macos` → `esp_logo` |
| `_Template/TemplateModel.h/.cpp` | 原样 | — |
| `Configs/Version.h` | 原样 | `Menu::Update()` 依赖其 `VERSION_*` 宏 |

### 4.7 资源与装配

| 目标 | 改动 | 说明 |
|---|---|---|
| `Resources/ResourcePool.h` | 重写 | 去掉 `Image_` 成员；`GetImage()` 改为声明；include 改指 `Utils/ResourceManager/` |
| `Resources/ResourcePool.cpp` | 重写 | 编译期注册 → 运行时加载，见 §5.3 |
| `app.h` | 重写 | 三个模式枚举 + `INIT_DONE()` 空宏 + `offsetof`/`container_of` 宏 + `App_Init()`/`App_UnInit()` 声明；AccountSystem 宏、Arduino 任务通知、`display_init` 声明已停用，见 §5.2 |
| `app.cpp` | 重写 | 见 §5.4（根样式 + immortal object） |
| `display_xknob.h/.cpp` | **新增** | 本工程 UI 入口，调用 `App_Init()` |
| `tools/v8img2png.py` | **新增** | v8 图片 `.c` → PNG |
| `Resources/Font/*`、`Resources/Image/*` | **新增** | 字体与图标 |

---

## 5. 改动说明

### 5.1 LVGL v8 → v9.5 API 改写（原写法一律以注释保留）

| 原写法（v8） | 现写法（v9.5） | 涉及文件 |
|---|---|---|
| `lv_mem_alloc` / `lv_mem_free` | `lv_malloc` / `lv_free` | `PM_Router.cpp`、`PM_State.cpp`、`PageBase.cpp`、`DataCenter/Account.cpp` |
| `timer->user_data` | `lv_timer_get_user_data(timer)` | `Menu.cpp`、`Template.cpp`、`DataCenter/Account.cpp` |
| `lv_event_get_current_target()` | `lv_event_get_current_target_obj()` | `PM_Drag.cpp` |
| `lv_event_send(obj, ...)` | `lv_obj_send_event(obj, ...)` | `PM_Drag.cpp` |
| `lv_event_get_target()` | `lv_event_get_target_obj()` | `Menu.cpp`、`Template.cpp` |
| `indev->driver->type` | `lv_indev_get_type(indev)` | `lv_obj_ext_func.cpp` |
| `lv_disp_set_bg_color(...)` | `lv_obj_set_style_bg_color(lv_screen_active(), ...)` | `ResourcePool.cpp` |
| **`LV_FS_MODE_WR \| LV_FS_MODE_RD`**（v8 可隐式转） | **`(lv_fs_mode_t)(LV_FS_MODE_WR \| LV_FS_MODE_RD)`**（v9 需强转） | `StorageService.cpp` |
| `lv_meter_*` | v9 已移除，需改 `lv_scale_*`（尚未涉及） | — |

> **`lv_fs_mode_t` 这条是一整类坑**：v8 里它是 `typedef uint8_t lv_fs_mode_t;`（枚举无名），
> v9 改成了 `typedef enum { LV_FS_MODE_WR, LV_FS_MODE_RD } lv_fs_mode_t;`。
> 于是「位或结果传给枚举参数」在 C++ 下不再隐式可转。**v8 里不少「枚举」其实是 typedef 成整数的**，
> 搬剩余 X-TRACK 代码时留意同类问题。

> v9.5 的 `lvgl.h` 会默认包含 `lv_api_map_v8.h`~`v9_4`（仅当定义了 `LV_DISABLE_API_MAPPING` 时才不包含；本工程未定义），因此 `lv_obj_del`、`lv_scr_act`、`lv_group_del`、`lv_img_set_src`、`lv_anim_set_time`、`lv_timer_del` 等大量 v8 名称**无需修改**即可编译。上表是兼容层仍未覆盖、必须手改的部分。

### 5.2 剥离 Arduino / ESP32 平台依赖（注释保留，不删除）

| 停用项 | 出现位置 |
|---|---|
| `#include <Arduino.h>` | `MenuModel.cpp`、`Template.cpp` |
| `#include "hal/motor.h"` | `Menu.cpp`、`app.h` |
| `HAL::power_off()` | `Menu.cpp`（仅此处） |
| `#include "hal/hal.h"`、`Accounts_Init()` | `app.cpp` |
| `#include "app/Accounts/Account_Master.h"` | `MenuModel.h`（`app.cpp` 里为去前缀的 `"Accounts/Account_Master.h"` 写法） |
| `#include "app/Pages/StatusBar/StatusBar.h"` | `Page.h`（`app.cpp` 里为 `"Pages/StatusBar/StatusBar.h"` 写法） |
| `Serial.printf` | 改为标准 `printf` |
| `ACCOUNT_SEND_NOTIFY_CMD`、`display_init()` | `app.h` |

> 新搬入的模块**无需剥离**，但**机制各不相同**（实测）：
> - **`Time`**：用了平台条件编译 —— `avr/pgmspace.h` 在 `#if defined(__AVR__)` 内、`Arduino.h`/`WProgram.h` 在 `#ifdef ARDUINO` 内，
>   且 `#else` 分支自带 `PROGMEM`/`pgm_read_byte`/`strcpy_P` 兜底（共 3 个文件含条件编译）。
> - **`DataCenter`**：仅 `DataCenterLog.h` 有 `#if !defined(ARDUINO) && DATA_CENTER_USE_LOG` 的日志分支（共 1 个文件）。
> - **`Filters` / `PointContainer` / `StorageService` / `lv_anim_label`**：**通篇不含任何平台条件编译** —— 它们本就不依赖 Arduino/ESP32，所以谈不上"走分支"。

### 5.3 重写与修正

| 文件 | 处理 |
|---|---|
| `Pages/Menu/MenuModel.{h,cpp}` | **退化为空壳**。原 `Init/Deinit/ChangeMotorMode` 依赖 AccountSystem 与电机 HAL，整段注释保留；本页 UI 并不消费电机数据 |
| `Resources/ResourcePool.cpp` | 由「编译期注册位图字体/图片描述符」改为「运行时加载」：字体走 Tiny TTF，图片走 `"A:"` 路径 + lodepng |
| `Pages/Menu/MenuView.cpp` | **修正上游笔误**：`"Surface Dial" "Control\n"` 两行相邻字面量被 C++ 拼接成 `"Surface DialControl\n"`，已补 `\n`（原写法以注释保留） |
| `app.h` | 删掉 AccountSystem 宏、Arduino 任务通知、`display_init` 声明 |
| `Utils/lv_anim_label/lv_anim_label.h` | **加 `#include "lvgl_private.h"`**，见下 |

> **v9 写自定义控件的必需条件**（`lv_anim_label` 的移植关键）：
> v9 把 `struct _lv_obj_t` 挪到 `core/lv_obj_private.h`、`struct _lv_obj_class_t` 挪到 `core/lv_obj_class_private.h`，
> **公开头里只有前向声明**（且 `LV_USE_PRIVATE_API = 0`）。而自定义控件需要
> ① 以 `lv_obj_t` 作为私有结构体首成员、② 用 `.constructor_cb/.instance_size/.base_class` 初始化类对象，
> 两者都要完整类型定义。
>
> 解法：`#include "lvgl_private.h"`（根级**一揽子聚合头**，第 48/51 行就包含上述两个私有头，且**无开关门**）。
> **必须放在声明结构体的那个头文件里，且在该结构体之前**——放在 `.c` 里无效，因为 `.c` 会先包含该头。

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

# 排除第三方库自带的「测试 / 示例 / CI / fuzzing」源码 —— 它们不该被编进本库。
# ArduinoJson 的 Utils/ArduinoJson/extras/ 下有 172 个 .c/.cpp（Catch2 单元测试、
# fuzzing 目标、ES-P-IDF CI 的 main.cpp），编译它们必然失败。
# 这与 X-TRACK 自身构建的做法一致：其 Linux/Makefile 用 filter-out 排除了
# USER/App/Utils/ArduinoJson 下的全部 .cpp（同样也排除了 lv_img_png 的 .cpp）。
# 注意：文件仍然完整保留在树里，只是不参与编译 —— 即「完整性」与「可编译」并存。
list(FILTER USR_SOURCES EXCLUDE REGEX "/Utils/ArduinoJson/")

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
#
#   另把 ArduinoJson 的 src/ 加进来：StorageService.cpp 里写的是
#   #include "ArduinoJson.h"，该名解析到 Utils/ArduinoJson/src/ArduinoJson.h
#   （与 X-TRACK 各构建的 include 配置一致）。
target_include_directories(xknob
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Utils/ArduinoJson/src
)

# 链接 LVGL 库
target_link_libraries(xknob PUBLIC lvgl)
```

**新增源文件**（如 `PageBase.cpp`）会被 `GLOB_RECURSE` 自动收集，**无需改 CMakeLists**。

**关于 `list(FILTER ...)` 这条例外**：`GLOB_RECURSE` 在补齐 ArduinoJson 完整性后命中 **200** 个 `.c/.cpp`，
排除后**参与编译的仍是 28 个**（被排除的 172 个正好是 `ArduinoJson/extras/` 下的全部 `.c/.cpp`）。
这是 X-TRACK 自己的做法（`filter-out`）在 CMake 下的等价写法。

### 6.2 根 `CMakeLists.txt`（接入 xknob 目标）

```cmake
add_subdirectory(${PROJECT_SOURCE_DIR}/main/xknob)  # 添加 xknob 子目录

target_link_libraries(main lvgl ... lv_demos_ext
                        usr_ui xknob)               # 追加 xknob
```

### 6.3 未改动

`main/usr_ui/CMakeLists.txt` 保持原样 —— `usr_ui` 库不受本次移植影响。

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

### 8.1 已在本轮补入（此前列为「未移植」）

`DataCenter`、`Filters`、`PointContainer`、`Time`、`StorageService`、`ArduinoJson`、`lv_anim_label`
—— 均按「完整性优先」搬入，见 §4.2～§4.4。

### 8.2 应用件（换项目即空壳，不搬）

| 上游路径 | 文件数 | 不搬原因 |
|---|---|---|
| `Pages/StatusBar/` | 2 | 常驻顶栏，但内容硬编码码表语义（卫星数/SD/电池/REC）；形态是框架件、内容是应用件 |
| `Pages/SystemInfos/`、`Pages/StartUp/` | 6 / 6 | 码表业务页 / 开机动画页 |
| `Pages/Playground/`、`Pages/Setting/` | 6 / 6 | 依赖电机 / WiFi+NVS；且含 `lv_meter`，v9 需改 `lv_scale` |
| `Pages/HASS/`、`Pages/SurfaceDial/` | 6 / 6 | 依赖 MQTT / BLE + 电机状态 |
| `Pages/Dialplate`、`LiveMap` | — | 依赖 DataProc / GPS / 地图瓦片 |
| `Common/DataProc/` 的 12 个 `DP_*.cpp` + `DP_LIST.inc` | — | 码表数据节点（`DataProc.cpp` 的 26 行节点管理器属通用机制，可另取） |
| `Common/HAL/` 的 GPS / SportStatus 结构体 | — | 码表传感器词汇表 |
| `USER/HAL/` 真机驱动 | 18 | AT32 引脚级实现（注：该目录在 `USER/` 下、**不在 `USER/App/` 内**） |
| `Utils/GPX_Parser/` | 2 | GPX 解析器 —— PC 桩 HAL 的 GPS 数据源，属应用件 |
| `Utils/MapConv/`、`Utils/TrackFilter/`、`Utils/GPX/` | 6 / 5 / 2 | 地图坐标转换（经纬度→瓦片）/ 轨迹抽稀裁剪 / GPX 文件写入 —— 均依赖经纬度与地图概念 |
| `Common/Music/` | 2 | 具体旋律数据（提示音曲谱表） |
| `Resources/Font/*.c`（5）、`Resources/Image/*.c`（41） | — | v8 位图格式，v9 不兼容 → 改用 Tiny TTF + PNG |

**以下模块属框架件/工具件、但本轮未搬**（需要时可按需另取，不在"不搬"之列）：

| 模块 | 文件数 | 归类 | 说明 |
|---|---|---|---|
| `Utils/lv_poly_line/` | 2 | 框架件（控件） | `lv_line` 折线增量缓冲（突破单 line 点数组上限） |
| `Utils/TileConv/` | 2 | 工具件 | 视口 ↔ 瓦片网格几何（纯 int32，零 LVGL / 零 FS） |
| `Utils/TonePlayer/` | 2 | 工具件 | 乐谱播放器（连 LVGL 都不依赖；发射端需调用方接蜂鸣器/PWM） |

### 8.3 明确不搬

| 模块 | 原因 |
|---|---|
| `Utils/lv_img_png/`（+PNGdec） | **依赖 v8 绘制管线**（`lv_draw_ctx_t`、`_lv_refr_get_disp_refreshing()`、`lv_disp_draw_buf_t`、`LV_COLOR_16_SWAP`），v9 全删；且上游默认关闭（`CONFIG_MAP_IMG_PNG_ENABLE=0`）。X-TRACK 自己的 Makefile 也把它 `filter-out` 了 |
| `Utils/new/` | ⚠️ 生效条件是 `#if defined(ARDUINO) \|\| defined(NDEBUG)`，而**我们的构建正是 Release 带 `-DNDEBUG`** → 它**会生效**，把全工程 C++ 堆挤进 LVGL 静态池，反而危险 |
| `Utils/lv_allocator/` | 全仓 0 include，且用了 v9 已删的 `lv_mem_*` |
| `Utils/Stream/`、`Utils/WString/` | Arduino 兼容层；`Stream` 唯一消费者 `GPX_Parser` 在 `USER/App` 内 0 引用；PC 上该用 `std::string`/`ifstream`/`strtod` |
| `lv_port/`（伞函数 + indev + fs_sdfat）、`benchmark.inc`、Simulator 的 win32drv | 已被 v9 内置 SDL/stdio 驱动替代 |

同时**删除了本工程原有的 `xknob_assets.{cpp,h}`**——X-Knob 自带 `ResourcePool`，不需要额外那套资源辅助。

---

## 9. 当前状态与已知限制

**已完成**：X-TRACK 版 PageManager（含 `Replace`/`SetRootDefaultStyle`/`onViewUnload`）+ ResourceManager + DataCenter + 工具库（Filters/PointContainer/Time/StorageService/ArduinoJson）+ lv_anim_label + X-Knob 的 Menu 与 `_Template`。
**入口**：`display_xknob()` —— 已接入 `main/src/main.c`（第 111 行调用，原 `test_ui()` 已注释）。分辨率 240×240。

| 说明 | 内容 |
|---|---|
| **新模块已编译就位、但尚未接入** | `DataCenter`/`Filters`/`PointContainer`/`Time`/`StorageService`/`lv_anim_label` 目前**没有任何外部文件 include 它们**，即「结构与编译正确性已就位，功能上还没被任何页面用到」。其中 `Filters` 是 header-only 模板，**只有被实例化时才做完整编译检查** |
| 二级页全部不可达 | 已安装页面只有 `Template` 与 `Menu`。Menu 的 5 个图标中，4 个分别 Push 到 `SurfaceDial`/`Playground`/`Hass`/`Setting`（均未安装），第 5 个 `System` 原本是关机、现仅打印日志；而 `Template` 虽已安装却**没有入口指向它**。此外 Menu 的页面 root 自身也绑了一个 Push 到 `Playground`（上游如此），故点击空白处同样无反应 |
| 输入以旋钮为主 | 鼠标滚轮 = 旋钮旋转，鼠标中键 = 按下（工程另有鼠标指针与键盘 indev，鼠标左键点击图标也会触发其事件）。这是上游设计——真机是无触摸的圆形旋钮屏 |
| 不支持拖拽滑动 | `PM_State.cpp` 清掉了页面 root 的 `LV_OBJ_FLAG_SCROLLABLE`，`MenuView.cpp` 也清掉了每个 item 的，指针拖拽因此找不到可滚动对象。交互模型是「焦点驱动滚动」 |
| 无实时数值 | 上游旋钮数值由消息框架驱动，砍掉电机后恒为 0 |

---

## 10. 移植约定

后续继续移植（X-TRACK 各页 / 其余模块）时沿用：

1. 移植工作**只允许修改 `main/xknob/CMakeLists.txt`**；根 `CMakeLists.txt` 的 xknob 接入（§6.2）与 `usr_ui/CMakeLists.txt` 均不由移植工作改动。
2. 移植文件里**停用的代码一律注释保留**，标注 `// 原: ...` 与 `// [移植改动] <原因>`，不删除。
3. **完整性优先**：整目录搬，不挑文件。若第三方库自带测试/示例会破坏编译，用**构建规则排除**（如 §6.1 的 `list(FILTER ...)`），而不是删文件。
4. 代码按「去掉一层 `app/`」的规则放到 `main/xknob/` 下，include 前缀相应为 `Utils/...`、`Resources/...`、`Configs/...`。
5. 只搬**框架件 / 工具件**（判定标准见 §2.2）；**应用件交给调用方**。
6. 对外入口统一为 `display_xknob()`。
7. **移入 `Utils/PageManager/` 的代码以 X-TRACK 版为准**；成员访问用下划线前缀（`_root`/`_Manager`/`_Name`）。
