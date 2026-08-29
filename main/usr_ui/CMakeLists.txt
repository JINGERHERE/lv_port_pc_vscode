# 自动添加源文件 # CONFIGURE_DEPENDS 构建时自动检测目录文件变化
file(GLOB USR_SOURCES CONFIGURE_DEPENDS *.c *.cpp)
file(GLOB USR_PRIV_SOURCES CONFIGURE_DEPENDS private/*.c private/*.cpp)

# 编译成静态库
add_library(usr_ui STATIC ${USR_SOURCES} ${USR_PRIV_SOURCES})

# 资源目录：编译期注入仓库根绝对路径（usr_assets.cpp 模拟器分支使用，与进程工作目录无关）
target_compile_definitions(usr_ui PRIVATE USR_ASSETS_PREFIX="${CMAKE_SOURCE_DIR}/main/usr_ui/assets/")

# 头文件目录
target_include_directories(usr_ui
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/priv_include
)

# 链接 LVGL 库
target_link_libraries(usr_ui PUBLIC lvgl)