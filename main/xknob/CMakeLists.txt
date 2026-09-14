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
