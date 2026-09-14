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
