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