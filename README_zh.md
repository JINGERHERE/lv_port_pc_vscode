# VSCode Simulator project for LVGL

## 分支切换
切换分支后若上游仓库未切换至指定版本，使用`git submodule update --init --recursive` 将所有 submodule 同步到正确的 commit


## macOS

- **安装必要依赖项**
1. macOS (Homebrew): `brew install sdl2 cmake make llvm`

2. VSCode: cmd+shift+p and run `Cmake: select a kit`, then `[Scan for kits]`

3. VSCode: then cmd+shift+p and run `Cmake: select a kit`, select the version of clang you just installed from homebrew (it should say `Using compilers C=/opt/homebrew/opt/llvm/bin/clang ...`).**其实就是选择 brew 安装的 clang，系统自带的 clang 版本较低。**


- **编译报错修改项**
    - 当前分支（release/v9.5）可直接编译运行
    - main 分支编译报错需修改（release 分支一般都能直接编译，main 分支不一定）:
        1. `lv_conf.h`: 修改 `#define LV_USE_SDL2 1`，启用 ThorVG库对矢量图形的支持
        2. `lv_conf.h`: 修改 `#define LV_SDL_INCLUDE_PATH "SDL2/SDL.h"` 为 `#define LV_SDL_INCLUDE_PATH "SDL.h"`
        3. `CMakeLists.txt`: 修改`target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR} ${SDL2_INCLUDE_DIRS})` 为 `target_include_directories(lvgl SYSTEM PUBLIC $<BUILD_INTERFACE:${SDL2_INCLUDE_DIRS}>)`



## Windows x64

- **安装必要依赖项**

1. 安装 `MinGW`：https://github.com/niXman/mingw-builds-binaries/releases/latest 选择 `x86_64_***_ucrt` 版本，解压后添加到环境变量 `PATH` 中


2. 安装 vcpkg：`git clone https://github.com/microsoft/vcpkg.git`克隆项目，进入项目目录，执行`./bootstrap-vcpkg.bat`安装 vcpkg。（按需使用`vcpkg integrate install`安装 MSBuild / Visual Studio 的全局集成，让 VS 打开的所有 MSBuild 工程自动找到 vcpkg 的库）

3. 新建 `./.vscode/settings.json`，添加内容（`DCMAKE_TOOLCHAIN_FILE`加载 vcpkg 工具链；通过`DVCPKG_TARGET_TRIPLET`和`DVCPKG_HOST_TRIPLET`明确安装的 ABI 版本和 ABI 构建类型）：
```json
{
    "cmake.generator": "MinGW Makefiles",
    "cmake.configureArgs": [
        "-DCMAKE_TOOLCHAIN_FILE=D:/DEV/vcpkg/scripts/buildsystems/vcpkg.cmake",
        "-DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic",
        "-DVCPKG_HOST_TRIPLET=x64-mingw-dynamic"
    ]
}
```
若需要静态链接 SDL2，则修改`"-DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic"`为`"-DVCPKG_TARGET_TRIPLET=x64-mingw-static"`

4. 新建 `./vcpkg.json`，添加内容：
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