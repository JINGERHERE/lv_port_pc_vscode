# VSCode Simulator project for LVGL


## 说明
本项目代码基于 **GCC 兼容工具链 + POSIX 接口**开发：链接 `pthread`、`m` 等 POSIX 库（FreeRTOS 使用 `GCC_POSIX` port），编译选项为 GNU 风格。Linux/macOS 原生满足，Windows 由 MinGW 补齐——因此在新的平台按「安装编译环境」装好工具链后即可直接编译。各平台编译参数统一由随仓库提交的 `CMakePresets.json` 管理：

- `CMakePresets.json`：定义三平台预设 `win-mingw` / `mac-clang` / `linux-clang`（生成器、编译器、triplet、编译类型、产物目录），通过 `${hostSystemName}` 条件自动只显示当前平台可用的预设
- **环境变量 `VCPKG_ROOT`**：每台机器设置一次，预设内通过 `$env{VCPKG_ROOT}` 引用 vcpkg 工具链，仓库文件中不出现绝对路径
- **编译类型不能留空**（仅影响 Windows 动态链接）：空编译类型会使链接端选中 debug 库（`SDL2d.dll`）、applocal 却去 release 目录找依赖，两边错位导致运行时找不到 DLL，故预设显式指定 `CMAKE_BUILD_TYPE=Release`
- **分支切换**：切换分支后若上游仓库未切换至指定版本，使用`git submodule update --init --recursive` 将所有 submodule 同步到正确的 commit
- **链接类型**：修改`CMakePresets.json`，`"VCPKG_TARGET_TRIPLET": "x64-mingw-dynamic"`为动态链接，`"VCPKG_TARGET_TRIPLET": "x64-mingw-static"`为静态链接


## 安装编译环境

通用要求：CMake ≥ 3.21（`CMakePresets.json` 使用 version 3 架构）、VSCode 安装 CMake Tools 插件

- **macOS**
    1. Homebrew安装必要依赖项：`brew install cmake make llvm`（系统自带 Apple clang 版本较低，预设显式使用 brew llvm 的 clang，路径 `/opt/homebrew/opt/llvm/bin/clang` 已写死在预设中，无需改 PATH）
    2. 安装 vcpkg：`git clone https://github.com/microsoft/vcpkg.git`克隆项目，进入项目目录，执行`./bootstrap-vcpkg.sh`下载 vcpkg 二进制文件。
    3. `~/.zshrc` 中添加 `export VCPKG_ROOT=<vcpkg目录路径>`

- **windows x64**
    1. 安装 cmake：https://cmake.org/download/
    2. 安装 `MinGW`：https://github.com/niXman/mingw-builds-binaries/releases/latest 选择 `x86_64_***_ucrt` 版本，解压后添加到环境变量 `PATH` 中
    3. 安装 vcpkg：`git clone https://github.com/microsoft/vcpkg.git`克隆项目，进入项目目录，执行`./bootstrap-vcpkg.bat`下载 vcpkg 二进制文件。添加系统环境变量`VCPKG_ROOT`，值为`D:/DEV/vcpkg`，`PATH`中添加`%VCPKG_ROOT%`

- **linux x64**
    1. 安装必要依赖项：`sudo apt update && sudo apt install -y build-essential cmake clang`
    2. 安装 vcpkg：`git clone https://github.com/microsoft/vcpkg.git`克隆项目，进入项目目录，执行`./bootstrap-vcpkg.sh`下载 vcpkg 二进制文件。
    3. `~/.bashrc` 中添加 `export VCPKG_ROOT=<vcpkg目录路径>`


## vcpkg.json
仓库根目录已包含 `./vcpkg.json`（manifest 模式依赖清单），内容如下：
```json
{
    "name": "lv-port-pc-vscode",
    "version-string": "1.0.0",
    "dependencies": ["sdl2"]
}

```

## 添加用户 UI
- 在固定目录下添加 ui 目录：`main/usr_ui`

- 在 `main/usr_ui` 目录下`CmakeLists.txt`（根据实际结构调整）：

    ```cmake
    # 自动添加源文件
    file(GLOB USR_SOURCES *.c *.cpp)
    file(GLOB USR_PRIV_SOURCES private/*.c private/*.cpp)

    # 编译成静态库
    add_library(usr_ui STATIC ${USR_SOURCES} ${USR_PRIV_SOURCES})

    # 头文件目录
    target_include_directories(usr_ui
        PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/priv_include
    )

    # 链接 LVGL 库
    target_link_libraries(usr_ui PUBLIC lvgl)
    ```

- 根目录`CMakeLists.txt` 添加：`add_subdirectory(${PROJECT_SOURCE_DIR}/main/usr_ui)`，约 90 行之后

- 根目录`CMakeLists.txt` 修改：`target_link_libraries(main lvgl lvgl::examples lvgl::demos lvgl::thorvg ${SDL2_LIBRARIES} m pthread)` 为 `target_link_libraries(main lvgl lvgl::examples lvgl::demos lvgl::thorvg ${SDL2_LIBRARIES} m pthread usr_ui)`，即追加链接 usr_ui 库