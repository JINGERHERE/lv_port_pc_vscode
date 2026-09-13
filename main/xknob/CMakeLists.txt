# 自动添加源文件 # CONFIGURE_DEPENDS 构建时自动检测目录文件变化
# 移植改动：GLOB → GLOB_RECURSE，以便递归收集 port/ 子目录下的 X-Knob 移植源码
# 原: file(GLOB USR_SOURCES CONFIGURE_DEPENDS *.c *.cpp)
file(GLOB_RECURSE USR_SOURCES CONFIGURE_DEPENDS *.c *.cpp)
file(GLOB_RECURSE USR_PRIV_SOURCES CONFIGURE_DEPENDS private/*.c private/*.cpp)

# 编译成静态库
add_library(xknob STATIC ${USR_SOURCES} ${USR_PRIV_SOURCES})

# 资源目录：编译期注入仓库根绝对路径（usr_assets.cpp 模拟器分支使用，与进程工作目录无关）
target_compile_definitions(xknob PRIVATE USR_ASSETS_PREFIX="${CMAKE_SOURCE_DIR}/main/xknob/assets/")

# 头文件目录
target_include_directories(xknob
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/priv_include
            # 移植改动：port/ 作为 X-Knob 移植代码的 include 根，
            # 使上游的 "app/..." / "hal/..." 绝对前缀与 "../Page.h" 相对包含原样生效
            ${CMAKE_CURRENT_SOURCE_DIR}/port
)

# 链接 LVGL 库
target_link_libraries(xknob PUBLIC lvgl)
