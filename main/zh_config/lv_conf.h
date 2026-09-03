/**
 * @file lv_conf.h
 * v9.5.0 配置文件
 */

/*
 * 将本文件复制为 `lv_conf.h` 使用：
 * 1. 直接放在 `lvgl` 文件夹旁边即可；
 * 2. 或放到任意其他位置，然后
 *    - 定义 `LV_CONF_INCLUDE_SIMPLE`；
 *    - 并将该路径加入头文件搜索路径。
 */

/* clang-format off */
#if 1 /* 启用以下配置内容 */
#ifndef LV_CONF_H
#define LV_CONF_H

/* 若需在此处包含头文件，务必放在 `__ASSEMBLY__` 条件内 */
#if  0 && defined(__ASSEMBLY__)
#include "my_include.h"
#endif

/*====================
   颜色设置
 *====================*/

/** 颜色位深：1 (I1)、8 (L8)、16 (RGB565)、24 (RGB888)、32 (XRGB8888) */
#define LV_COLOR_DEPTH 32

/*=========================
   标准库封装设置
 *=========================*/

/** 可选值
 * - LV_STDLIB_BUILTIN:     LVGL 内置实现
 * - LV_STDLIB_CLIB:        标准 C 函数，如 malloc、strlen 等
 * - LV_STDLIB_MICROPYTHON: MicroPython 实现
 * - LV_STDLIB_RTTHREAD:    RT-Thread 实现
 * - LV_STDLIB_CUSTOM:      由外部自行实现这些函数
 */
#define LV_USE_STDLIB_MALLOC    LV_STDLIB_BUILTIN

/** 可选值
 * - LV_STDLIB_BUILTIN:     LVGL 内置实现
 * - LV_STDLIB_CLIB:        标准 C 函数，如 malloc、strlen 等
 * - LV_STDLIB_MICROPYTHON: MicroPython 实现
 * - LV_STDLIB_RTTHREAD:    RT-Thread 实现
 * - LV_STDLIB_CUSTOM:      由外部自行实现这些函数
 */
#define LV_USE_STDLIB_STRING    LV_STDLIB_BUILTIN

/** 可选值
 * - LV_STDLIB_BUILTIN:     LVGL 内置实现
 * - LV_STDLIB_CLIB:        标准 C 函数，如 malloc、strlen 等
 * - LV_STDLIB_MICROPYTHON: MicroPython 实现
 * - LV_STDLIB_RTTHREAD:    RT-Thread 实现
 * - LV_STDLIB_CUSTOM:      由外部自行实现这些函数
 */
#define LV_USE_STDLIB_SPRINTF   LV_STDLIB_BUILTIN

#define LV_STDINT_INCLUDE       <stdint.h>
#define LV_STDDEF_INCLUDE       <stddef.h>
#define LV_STDBOOL_INCLUDE      <stdbool.h>
#define LV_INTTYPES_INCLUDE     <inttypes.h>
#define LV_LIMITS_INCLUDE       <limits.h>
#define LV_STDARG_INCLUDE       <stdarg.h>

#if LV_USE_STDLIB_MALLOC == LV_STDLIB_BUILTIN
    /** `lv_malloc()` 可用内存大小，单位字节（≥ 2kB） */
    #define LV_MEM_SIZE (1024 * 1024)

    /** `lv_malloc()` 内存池可扩展的大小，单位字节 */
    #define LV_MEM_POOL_EXPAND_SIZE 0

    /** 为内存池指定地址，而非按普通数组分配，也可指向外部 SRAM */
    #define LV_MEM_ADR 0     /**< 0：不使用 */
    /* 也可不指定地址，而是提供一个内存分配函数，由 LVGL 调用来获取内存池，如 my_malloc */
    #if LV_MEM_ADR == 0
        #undef LV_MEM_POOL_INCLUDE
        #undef LV_MEM_POOL_ALLOC
    #endif
#endif  /*LV_USE_STDLIB_MALLOC == LV_STDLIB_BUILTIN*/

/*====================
   硬件抽象层（HAL）设置
 *====================*/

/** 显示刷新、输入设备读取与动画步进 的默认周期 */
#define LV_DEF_REFR_PERIOD  33      /**< [ms] */

/** 默认 DPI（每英寸点数）。用于初始化各类默认尺寸，如控件尺寸、样式内边距。
 * （并不关键，调整它即可改变各类默认尺寸与间距。） */
#define LV_DPI_DEF 130              /**< [px/inch] */

/*=================
 * 操作系统
 *=================*/
/** 选择使用的操作系统。可选值：
 * - LV_OS_NONE
 * - LV_OS_PTHREAD
 * - LV_OS_FREERTOS
 * - LV_OS_CMSIS_RTOS2
 * - LV_OS_RTTHREAD
 * - LV_OS_WINDOWS
 * - LV_OS_MQX
 * - LV_OS_SDL2
 * - LV_OS_CUSTOM */
#define LV_USE_OS   LV_OS_NONE

#if LV_USE_OS == LV_OS_CUSTOM
    #define LV_OS_CUSTOM_INCLUDE <stdint.h>
#endif
#if LV_USE_OS == LV_OS_FREERTOS
    /*
     * 用直接通知解除 RTOS 任务阻塞，比经由二值信号量等中间对象快 45%，且占用 RAM 更少。
     * 仅当事件只可能由一个任务接收时，才能使用 RTOS 任务通知。
     */
    #define LV_USE_FREERTOS_TASK_NOTIFY 1
#endif

/*========================
 * 渲染配置
 *========================*/

/** 所有图层和图像的行跨度（stride）按此字节数对齐 */
#define LV_DRAW_BUF_STRIDE_ALIGN                1

/** draw_buf 起始地址按此字节数对齐 */
#define LV_DRAW_BUF_ALIGN                       4

/** 变换时使用矩阵。
 * 前提条件：
 * - `LV_USE_MATRIX = 1`；
 * - 渲染引擎需支持 3x3 矩阵变换。 */
#define LV_DRAW_TRANSFORM_USE_MATRIX            0

/* 当控件的 `style_opa < 255`（注意不是 `bg_opa`、`text_opa` 等），或混合模式不是 NORMAL 时，
 * 会先缓冲到"simple"图层再渲染。控件可分小块缓冲。
 * "变换图层"（设置了 `transform_angle/zoom` 时）需要更大的缓冲，
 * 且无法分块绘制。 */

/** simple 图层分块的目标缓冲大小 */
#define LV_DRAW_LAYER_SIMPLE_BUF_SIZE    (24 * 1024)    /**< [bytes]*/

/* 限制 simple 与变换图层可分配的最大内存。
 * 至少应达到 `LV_DRAW_LAYER_SIMPLE_BUF_SIZE` 的大小；若还用到变换图层，
 * 则需足以容纳最大的控件（宽 x 高 x 4 的区域）。
 * 设为 0 表示不限制。 */
#define LV_DRAW_LAYER_MAX_MEMORY 0  /**< 默认不限制 [bytes]*/

/** 绘制线程的栈大小。
 * 注意：若启用了 FreeType 或 ThorVG，建议设为 32KB 或更大。
 */
#define LV_DRAW_THREAD_STACK_SIZE    (8 * 1024)         /**< [bytes]*/

/** 绘制任务的线程优先级。
 *  数值越大优先级越高。
 *  可使用 lv_os.h 中 lv_thread_prio_t 枚举的值：LV_THREAD_PRIO_LOWEST、
 *  LV_THREAD_PRIO_LOW、LV_THREAD_PRIO_MID、LV_THREAD_PRIO_HIGH、LV_THREAD_PRIO_HIGHEST
 *  请确保优先级数值与具体 OS 的优先级档位相匹配。
 *  在优先级档位有限的系统上（如 FreeRTOS），调高此值可提升渲染性能，
 *  但可能导致其他任务得不到调度。 */
#define LV_DRAW_THREAD_PRIO LV_THREAD_PRIO_HIGH

#define LV_USE_DRAW_SW 1
#if LV_USE_DRAW_SW == 1
    /*
     * 可按需关闭部分颜色格式支持，以减小代码体积。
     * 注意：部分功能会在内部使用特定颜色格式，例如：
     * - 渐变使用 RGB888
     * - 带透明度的位图可能使用 ARGB8888
     */
    #define LV_DRAW_SW_SUPPORT_RGB565       1
    #define LV_DRAW_SW_SUPPORT_RGB565_SWAPPED       1
    #define LV_DRAW_SW_SUPPORT_RGB565A8     1
    #define LV_DRAW_SW_SUPPORT_RGB888       1
    #define LV_DRAW_SW_SUPPORT_XRGB8888     1
    #define LV_DRAW_SW_SUPPORT_ARGB8888     1
    #define LV_DRAW_SW_SUPPORT_ARGB8888_PREMULTIPLIED 1
    #define LV_DRAW_SW_SUPPORT_L8           1
    #define LV_DRAW_SW_SUPPORT_AL88         1
    #define LV_DRAW_SW_SUPPORT_A8           1
    #define LV_DRAW_SW_SUPPORT_I1           1

    /* 索引颜色格式（I1）下，判定像素为"点亮"的亮度阈值 */
    #define LV_DRAW_SW_I1_LUM_THRESHOLD 127

    /** 设置绘制单元数量。
     *  - 大于 1 要求在 `LV_USE_OS` 中启用操作系统。
     *  - 大于 1 表示多线程并行渲染屏幕。 */
    #define LV_DRAW_SW_DRAW_UNIT_CNT    1

    /** 使用 Arm-2D 加速软件渲染。 */
    #define LV_USE_DRAW_ARM2D_SYNC      0

    /** 启用原生 Helium 汇编参与编译。 */
    #define LV_USE_NATIVE_HELIUM_ASM    0

    /**
     * - 0：使用简单渲染器，仅能绘制带渐变的简单矩形、图像、文字与直线。
     * - 1：使用复杂渲染器，还可绘制圆角、阴影、斜线与圆弧。 */
    #define LV_DRAW_SW_COMPLEX          1

    #if LV_DRAW_SW_COMPLEX == 1
        /** 允许缓冲部分阴影计算结果。
         *  LV_DRAW_SW_SHADOW_CACHE_SIZE 是可缓冲的最大阴影尺寸，阴影尺寸指
         *  `shadow_width + radius`。缓存的 RAM 开销为 LV_DRAW_SW_SHADOW_CACHE_SIZE 的平方。 */
        #define LV_DRAW_SW_SHADOW_CACHE_SIZE 0

        /** 设置最多缓存多少个圆的数据。
         *  为抗锯齿会保存 1/4 圆弧的周长数据。
         *  每个圆占用 `radius * 4` 字节（保存最常用的那些半径）。
         *  - 0：禁用缓存 */
        #define LV_DRAW_SW_CIRCLE_CACHE_SIZE 4
    #endif

    #define  LV_USE_DRAW_SW_ASM     LV_DRAW_SW_ASM_NONE

    #if LV_USE_DRAW_SW_ASM == LV_DRAW_SW_ASM_CUSTOM
        #define  LV_DRAW_SW_ASM_CUSTOM_INCLUDE ""
    #endif

    /** 启用软件绘制复杂渐变：斜向线性、径向与锥形渐变 */
    #define LV_USE_DRAW_SW_COMPLEX_GRADIENTS    1

#endif

/* 使用 TSi（Think Silicon）的 NemaGFX */
#define LV_USE_NEMA_GFX 0

#if LV_USE_NEMA_GFX
    /** 选择使用哪套 NemaGFX 静态库头文件。可选值：
     * - LV_NEMA_LIB_NONE           LV_NEMA_LIB_M33_REVC 的别名
     * - LV_NEMA_LIB_M33_REVC
     * - LV_NEMA_LIB_M33_NEMAPVG
     * - LV_NEMA_LIB_M55
     * - LV_NEMA_LIB_M7
     * 还需自行在 libs/nema_gfx/lib/core/ 中链接对应的静态库
     */
    #define LV_USE_NEMA_LIB LV_NEMA_LIB_NONE

    /** 选择使用哪个 NemaGFX HAL。可选值：
     * - LV_NEMA_HAL_CUSTOM
     * - LV_NEMA_HAL_STM32 */
    #define LV_USE_NEMA_HAL LV_NEMA_HAL_CUSTOM
    #if LV_USE_NEMA_HAL == LV_NEMA_HAL_STM32
        #define LV_NEMA_STM32_HAL_INCLUDE <stm32u5xx_hal.h>

        /** 若需将 GPU 内存放在特定区域（例如访问不被缓存的位置），
         * 可将其设为类似 __attribute__((section("Nemagfx_Memory_Pool_Buffer")))
         * 的值，并在链接脚本中定义相应段。
         */
        #define LV_NEMA_STM32_HAL_ATTRIBUTE_POOL_MEM
    #endif

    /* 启用矢量图形操作。仅在存在 NemaVG 库时可用 */
    #define LV_USE_NEMA_VG 0
    #if LV_USE_NEMA_VG
        /* 定义应用分辨率，用于 VG 相关的缓冲区分配 */
        #define LV_NEMA_GFX_MAX_RESX 800
        #define LV_NEMA_GFX_MAX_RESY 600
    #endif
#endif

/** 在 iMX RTxxx 平台上使用 NXP 的 PXP。 */
#define LV_USE_PXP 0

#if LV_USE_PXP
    /** 使用 PXP 进行绘制。*/
    #define LV_USE_DRAW_PXP 1

    /** 使用 PXP 旋转显示。*/
    #define LV_USE_ROTATE_PXP 0

    #if LV_USE_DRAW_PXP && LV_USE_OS
        /** 为 PXP 处理启用额外的绘制线程。*/
        #define LV_USE_PXP_DRAW_THREAD 1
    #endif

    /** 启用 PXP 断言。 */
    #define LV_USE_PXP_ASSERT 0
#endif

/** 在 MPU 平台上使用 NXP 的 G2D。 */
#define LV_USE_G2D 0

#if LV_USE_G2D
    /** 使用 G2D 进行绘制。 **/
    #define LV_USE_DRAW_G2D 1

    /** 使用 G2D 旋转显示。 **/
    #define LV_USE_ROTATE_G2D 0

    /** G2D 绘制单元可缓存的缓冲区最大数量。
     *  含帧缓冲与资源。 */
    #define LV_G2D_HASH_TABLE_SIZE 50

    #if LV_USE_DRAW_G2D && LV_USE_OS
        /** 为 G2D 处理启用额外的绘制线程。*/
        #define LV_USE_G2D_DRAW_THREAD 1
    #endif

    /** 启用 G2D 断言。 */
    #define LV_USE_G2D_ASSERT 0
#endif

/** 在 RA 平台上使用 Renesas 的 Dave2D。 */
#define LV_USE_DRAW_DAVE2D 0

/** 使用缓存的 SDL 纹理进行绘制 */
#define LV_USE_DRAW_SDL 0

/** 使用 VG-Lite GPU。 */
#define LV_USE_DRAW_VG_LITE 0
#if LV_USE_DRAW_VG_LITE
    /** 启用 VG-Lite 自定义的外部 'gpu_init()' 函数 */
    #define LV_VG_LITE_USE_GPU_INIT 0

    /** 启用 VG-Lite 断言。 */
    #define LV_VG_LITE_USE_ASSERT 0

    /** VG-Lite 刷新提交的触发阈值。GPU 会尝试将这么多个绘制任务批量提交。 */
    #define LV_VG_LITE_FLUSH_MAX_COUNT 8

    /** 启用边框模拟阴影。
     *  注意：通常可提升性能，
     *  但不保证与软件渲染质量一致。 */
    #define LV_VG_LITE_USE_BOX_SHADOW 1

    /** VG-Lite 渐变的最大缓存数量。
     *  @note 单张渐变图像占用 4K 字节内存。 */
    #define LV_VG_LITE_GRAD_CACHE_CNT 32

    /** VG-Lite 描边的最大缓存数量。 */
    #define LV_VG_LITE_STROKE_CACHE_CNT 32

    /** VG-Lite 非对齐位图字体的最大缓存数量。 */
    #define LV_VG_LITE_BITMAP_FONT_CACHE_CNT 256

    /** 移除 VLC_OP_CLOSE 路径指令（NXP 的临时规避方案） **/
    #define LV_VG_LITE_DISABLE_VLC_OP_CLOSE 0

    /** 禁用位块传输（blit）矩形偏移，以规避某些硬件错误。 */
    #define LV_VG_LITE_DISABLE_BLIT_RECT_OFFSET 0

    /** 对某些旧版本驱动禁用线性渐变扩展。 */
    #define LV_VG_LITE_DISABLE_LINEAR_GRADIENT_EXT 0

    /** 路径转储打印的最大长度（以点数计） */
    #define LV_VG_LITE_PATH_DUMP_MAX_LEN 1000

    /** 启用使用 LVGL 内置的 vg_lite 驱动 */
    #define LV_USE_VG_LITE_DRIVER  0
    #if LV_USE_VG_LITE_DRIVER
        /** 用于选择正确的 GPU 系列文件夹，可选值：gc255、gc355、gc555 */
        #define LV_VG_LITE_HAL_GPU_SERIES gc255

        /** 用于选择正确的 GPU 版本头文件，取决于厂商 */
        #define LV_VG_LITE_HAL_GPU_REVISION 0x40

        /** GPU IP 的基地址，取决于 SoC，
         *  默认值适用于 NXP 设备 */
        #define LV_VG_LITE_HAL_GPU_BASE_ADDRESS 0x40240000
    #endif /*LV_USE_VG_LITE_DRIVER*/

    /** 使用 ThorVG（软件矢量库）作为 VG-Lite 驱动，以便在 PC 上测试 VGLite
     *  需要：LV_USE_THORVG_INTERNAL 或 LV_USE_THORVG_EXTERNAL */
    #define LV_USE_VG_LITE_THORVG   0
    #if LV_USE_VG_LITE_THORVG
        /** 启用 LVGL 混合模式支持 */
        #define LV_VG_LITE_THORVG_LVGL_BLEND_SUPPORT 0

        /** 启用 YUV 颜色格式支持 */
        #define LV_VG_LITE_THORVG_YUV_SUPPORT 0

        /** 启用线性渐变扩展支持 */
        #define LV_VG_LITE_THORVG_LINEAR_GRADIENT_EXT_SUPPORT 0

        /** 启用 16 像素对齐 */
        #define LV_VG_LITE_THORVG_16PIXELS_ALIGN 1

        /** 缓冲区地址对齐 */
        #define LV_VG_LITE_THORVG_BUF_ADDR_ALIGN 64

        /** 启用多线程渲染 */
        #define LV_VG_LITE_THORVG_THREAD_RENDER 0
    #endif /*LV_USE_VG_LITE_THORVG*/
#endif

/** 使用 STM32 DMA2D 加速混合、填充等操作 */
#define LV_USE_DRAW_DMA2D 0
#if LV_USE_DRAW_DMA2D
    #define LV_DRAW_DMA2D_HAL_INCLUDE "stm32h7xx_hal.h"

    /* 若启用，用户需在收到 DMA2D 全局中断时
     * 调用 `lv_draw_dma2d_transfer_complete_interrupt_handler`
     */
    #define LV_USE_DRAW_DMA2D_INTERRUPT 0
#endif

/** 使用缓存的 OpenGLES 纹理进行绘制。需要 LV_USE_OPENGLES */
#define LV_USE_DRAW_OPENGLES 0
#if LV_USE_DRAW_OPENGLES
    #define LV_DRAW_OPENGLES_TEXTURE_CACHE_COUNT 64
#endif

/** 使用乐鑫 PPA 加速器进行绘制 */
#define LV_USE_PPA  0
#if LV_USE_PPA
    #define LV_USE_PPA_IMG      0
    #define LV_PPA_BURST_LENGTH    128
#endif

/* 使用 EVE FT81X GPU。 */
#define LV_USE_DRAW_EVE 0
#if LV_USE_DRAW_EVE
    /* EVE_GEN 取值：2、3 或 4 */
    #define LV_DRAW_EVE_EVE_GENERATION 4

    /* 单次 SPI 传输前可缓冲的最大字节数。
     * 设为 0 可禁用写入缓冲。
     */
    #define LV_DRAW_EVE_WRITE_BUFFER_SIZE 2048
#endif

/** 使用 NanoVG 渲染器
 * - 需要 LV_USE_NANOVG、LV_USE_MATRIX。
 */
#define LV_USE_DRAW_NANOVG 0
#if LV_USE_DRAW_NANOVG
    /** 为 NanoVG 选择 OpenGL 后端：
     * - LV_NANOVG_BACKEND_GL2:   OpenGL 2.0
     * - LV_NANOVG_BACKEND_GL3:   OpenGL 3.0+
     * - LV_NANOVG_BACKEND_GLES2: OpenGL ES 2.0
     * - LV_NANOVG_BACKEND_GLES3: OpenGL ES 3.0+
     */
    #define LV_NANOVG_BACKEND   LV_NANOVG_BACKEND_GLES2

    /** 图像纹理缓存数量。 */
    #define LV_NANOVG_IMAGE_CACHE_CNT 128

    /** 字符纹理缓存数量。 */
    #define LV_NANOVG_LETTER_CACHE_CNT 512
#endif

/*=======================
 * 功能配置
 *=======================*/

/*-------------
 * 日志
 *-----------*/

/** 启用日志模块 */
#define LV_USE_LOG 1
#if LV_USE_LOG
    /** 将其设为以下日志详细级别之一：
     *  - LV_LOG_LEVEL_TRACE    记录详细信息。
     *  - LV_LOG_LEVEL_INFO     记录重要事件。
     *  - LV_LOG_LEVEL_WARN     记录异常但未造成问题的情况。
     *  - LV_LOG_LEVEL_ERROR    仅记录可能导致系统故障的严重问题。
     *  - LV_LOG_LEVEL_USER     仅记录用户自定义的日志。
     *  - LV_LOG_LEVEL_NONE     不记录任何日志。 */
    #define LV_LOG_LEVEL LV_LOG_LEVEL_WARN

    /** - 1：使用 'printf' 打印日志；
     *  - 0：用户需通过 `lv_log_register_print_cb()` 注册回调。 */
    #define LV_LOG_PRINTF 1

    /** 设置打印日志的回调。
     *  如 `my_print`，原型应为 `void my_print(lv_log_level_t level, const char * buf)`。
     *  可被 `lv_log_register_print_cb` 覆盖。 */
    //#define LV_LOG_PRINT_CB

    /** - 1：启用打印时间戳；
     *  - 0：禁用打印时间戳。 */
    #define LV_LOG_USE_TIMESTAMP 1

    /** - 1：打印日志所在的文件与行号；
     *  - 0：不打印日志所在的文件与行号。 */
    #define LV_LOG_USE_FILE_LINE 1

    /* 在产生大量日志的模块中启用/禁用 LV_LOG_TRACE。 */
    #define LV_LOG_TRACE_MEM        1   /**< 启用/禁用内存操作的跟踪日志。 */
    #define LV_LOG_TRACE_TIMER      1   /**< 启用/禁用定时器操作的跟踪日志。 */
    #define LV_LOG_TRACE_INDEV      1   /**< 启用/禁用输入设备操作的跟踪日志。 */
    #define LV_LOG_TRACE_DISP_REFR  1   /**< 启用/禁用显示重绘操作的跟踪日志。 */
    #define LV_LOG_TRACE_EVENT      1   /**< 启用/禁用事件分发逻辑的跟踪日志。 */
    #define LV_LOG_TRACE_OBJ_CREATE 1   /**< 启用/禁用对象创建的跟踪日志（含核心 `obj` 创建与所有控件）。 */
    #define LV_LOG_TRACE_LAYOUT     1   /**< 启用/禁用 flex 与 grid 布局操作的跟踪日志。 */
    #define LV_LOG_TRACE_ANIM       1   /**< 启用/禁用动画逻辑的跟踪日志。 */
    #define LV_LOG_TRACE_CACHE      1   /**< 启用/禁用缓存操作的跟踪日志。 */
#endif  /*LV_USE_LOG*/

/*-------------
 * 断言
 *-----------*/

/* 操作失败或发现无效数据时触发断言失败。
 * 若启用了 LV_USE_LOG，断言失败时会打印错误信息。 */
#define LV_USE_ASSERT_NULL          1   /**< 检查参数是否为 NULL。（极快，推荐） */
#define LV_USE_ASSERT_MALLOC        1   /**< 检查内存是否分配成功。（极快，推荐） */
#define LV_USE_ASSERT_STYLE         1
#define LV_USE_ASSERT_MEM_INTEGRITY 1
#define LV_USE_ASSERT_OBJ           1

/** 断言触发时执行自定义处理，例如重启 MCU。 */
#define LV_ASSERT_HANDLER_INCLUDE <stdint.h>
#define LV_ASSERT_HANDLER while(1);     /**< 默认为死循环停机 */

/*-------------
 * 调试
 *-----------*/

/** 1：在重绘区域上绘制随机颜色的矩形。 */
#define LV_USE_REFR_DEBUG 0

/** 1：为 ARGB 图层叠加红色蒙层，为 RGB 图层叠加绿色蒙层*/
#define LV_USE_LAYER_DEBUG 0

/** 1：为调试添加以下行为：
 *  - 用不同颜色为各绘制单元（draw_unit）的任务绘制蒙层。
 *  - 在白底上绘制绘制单元的编号。
 *  - 对图层，则在黑底上绘制绘制单元的编号。 */
#define LV_USE_PARALLEL_DRAW_DEBUG 0

/*-------------
 * 其他
 *-----------*/

#define LV_ENABLE_GLOBAL_CUSTOM 0
#if LV_ENABLE_GLOBAL_CUSTOM
    /** 自定义 'lv_global' 函数所需包含的头文件 */
    #define LV_GLOBAL_CUSTOM_INCLUDE <stdint.h>
#endif

/** 默认缓存大小，单位字节。
 *  供 `lv_lodepng` 等图像解码器在内存中保留已解码图像使用。
 *  若不为 0，缓存满时解码器将解码失败。
 *  若为 0，则不启用缓存功能，已解码的内存
 *  会在使用后立即释放。 */
#define LV_CACHE_DEF_SIZE       0

/** 图像头缓存条目的默认数量。该缓存用于存储图像头，
 *  逻辑与 `LV_CACHE_DEF_SIZE` 类似，只是作用于图像头。 */
#define LV_IMAGE_HEADER_CACHE_DEF_CNT 0

/** 每个渐变允许的色标数量。调大可支持更多色标。
 *  每增加一个色标多占 (sizeof(lv_color_t) + 1) 字节。 */
#define LV_GRADIENT_MAX_STOPS   2

/** 调整颜色混合函数的舍入方式。GPU 计算颜色混合（混合运算）的方式可能不同。
 *  - 0:   向下取整，
 *  - 64:  自 x.75 起向上取整，
 *  - 128: 自 0.5 起向上取整，
 *  - 192: 自 x.25 起向上取整，
 *  - 254: 一律向上取整 */
#define LV_COLOR_MIX_ROUND_OFS  0

/** 为每个 `lv_obj_t` 增加 2 个 32 位变量，以加速样式属性读取 */
#define LV_OBJ_STYLE_CACHE      1

/** 为 `lv_obj_t` 增加 `id` 字段 */
#define LV_USE_OBJ_ID           0

/** 启用控件名称支持 */
#define LV_USE_OBJ_NAME         0

/** 对象创建时自动分配 ID */
#define LV_OBJ_ID_AUTO_ASSIGN   LV_USE_OBJ_ID

/** 使用内置的对象 ID 处理函数：
* - lv_obj_assign_id:       控件创建时调用。为每个控件类使用独立计数器作为 ID。
* - lv_obj_id_compare:      比较 ID 是否与请求值匹配。
* - lv_obj_stringify_id:    返回字符串形式的标识符，如 "button3"。
* - lv_obj_free_id:         空操作，ID 不涉及内存分配。
* 若禁用，这些函数需由用户自行实现。*/
#define LV_USE_OBJ_ID_BUILTIN   1

/** 使用对象属性的 set/get API。 */
#define LV_USE_OBJ_PROPERTY 0

/** 启用属性名支持。 */
#define LV_USE_OBJ_PROPERTY_NAME 1

/* 启用多点触控手势识别功能 */
/* 手势识别需要使用浮点运算 */
#define LV_USE_GESTURE_RECOGNITION 0

/*=====================
 *  编译器设置
 *====================*/

/** 大端系统请设为 1 */
#define LV_BIG_ENDIAN_SYSTEM 0

/** 为 `lv_tick_inc` 函数定义自定义属性 */
#define LV_ATTRIBUTE_TICK_INC

/** 为 `lv_timer_handler` 函数定义自定义属性 */
#define LV_ATTRIBUTE_TIMER_HANDLER

/** 为 `lv_display_flush_ready` 函数定义自定义属性 */
#define LV_ATTRIBUTE_FLUSH_READY

/** VG_LITE 缓冲区按此字节数对齐。
 *  @note vglite_src_buf_aligned() 用此值校验传入缓冲指针的对齐。 */
#define LV_ATTRIBUTE_MEM_ALIGN_SIZE 1

/** 添加到需要内存对齐之处（-Os 下数据默认可能不对齐边界）。
 *  例如 __attribute__((aligned(4)))*/
#define LV_ATTRIBUTE_MEM_ALIGN

/** 标记大型常量数组的属性，如字体位图 */
#define LV_ATTRIBUTE_LARGE_CONST

/** 在 RAM 中声明大型数组的编译器前缀 */
#define LV_ATTRIBUTE_LARGE_RAM_ARRAY

/** 将性能关键函数放入更快的存储器（如 RAM） */
#define LV_ATTRIBUTE_FAST_MEM

/** 将整型常量导出到绑定层。此宏用于 LV_<CONST> 形式的常量，
 *  使其同时出现在 MicroPython 等 LVGL 绑定 API 中。 */
#define LV_EXPORT_CONST_INT(int_value) struct _silence_gcc_warning  /**< 默认值仅为消除 GCC 警告 */

/** 所有全局 extern 数据加此前缀 */
#define LV_ATTRIBUTE_EXTERN_DATA

/** 使用 `float` 作为 `lv_value_precise_t` */
#define LV_USE_FLOAT            1

/** 启用矩阵支持
 *  - 需要 `LV_USE_FLOAT = 1` */
#define LV_USE_MATRIX           1

/** 在 `lvgl.h` 中包含 `lvgl_private.h`，以便默认访问内部数据与函数 */
#ifndef LV_USE_PRIVATE_API
    #define LV_USE_PRIVATE_API  0
#endif

/*==================
 *   字体
 *===================*/

/* Montserrat 字体，ASCII 范围及部分符号，bpp = 4
 * https://fonts.google.com/specimen/Montserrat */
#define LV_FONT_MONTSERRAT_8  0
#define LV_FONT_MONTSERRAT_10 0
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_26 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_30 1
#define LV_FONT_MONTSERRAT_32 1
#define LV_FONT_MONTSERRAT_34 1
#define LV_FONT_MONTSERRAT_36 1
#define LV_FONT_MONTSERRAT_38 1
#define LV_FONT_MONTSERRAT_40 1
#define LV_FONT_MONTSERRAT_42 1
#define LV_FONT_MONTSERRAT_44 1
#define LV_FONT_MONTSERRAT_46 1
#define LV_FONT_MONTSERRAT_48 1

/* 特殊字体示例 */
#define LV_FONT_MONTSERRAT_28_COMPRESSED    1
#define LV_FONT_DEJAVU_16_PERSIAN_HEBREW    1
#define LV_FONT_SOURCE_HAN_SANS_SC_14_CJK   0  /**< 1338 个最常用 CJK 部首 */
#define LV_FONT_SOURCE_HAN_SANS_SC_16_CJK   0  /**< 1338 个最常用 CJK 部首 */

/** 像素级精确的等宽字体 */
#define LV_FONT_UNSCII_8  1
#define LV_FONT_UNSCII_16 0

/** 可在此声明自定义字体。
 *
 *  其中任何字体也可设为默认字体，并全局可用。示例：
 *
 *  @code
 *  #define LV_FONT_CUSTOM_DECLARE   LV_FONT_DECLARE(my_font_1) LV_FONT_DECLARE(my_font_2)
 *  @endcode
 */
#define LV_FONT_CUSTOM_DECLARE

/** 务必设置默认字体 */
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/** 启用对大字体和/或多字符字体的支持。
 *  上限取决于字体大小、字体与 bpp。
 *  若某字体需要此支持而未启用，会触发编译错误。 */
#define LV_FONT_FMT_TXT_LARGE 0

/** 启用/禁用压缩字体支持。 */
#define LV_USE_FONT_COMPRESSED 0

/** 找不到字形描述时，绘制占位符。 */
#define LV_USE_FONT_PLACEHOLDER 1

/*=================
 *  文本设置
 *=================*/

/**
 * 选择字符串的字符编码。
 * IDE 或编辑器应使用相同的字符编码。
 * - LV_TXT_ENC_UTF8
 * - LV_TXT_ENC_ASCII
 */
#define LV_TXT_ENC LV_TXT_ENC_UTF8

/** 渲染文本时，可在这些字符处断行（换行）。 */
#define LV_TXT_BREAK_CHARS " ,.;:-_)]}"

/** 单词达到此长度时，在"最美观"的位置断行。
 *  设为 <= 0 可禁用。 */
#define LV_TXT_LINE_BREAK_LONG_LEN 0

/** 长单词断行前，一行至少要放的字符数。
 *  依赖 LV_TXT_LINE_BREAK_LONG_LEN。 */
#define LV_TXT_LINE_BREAK_LONG_PRE_MIN_LEN 3

/** 长单词断行后，一行至少要放的字符数。
 *  依赖 LV_TXT_LINE_BREAK_LONG_LEN。 */
#define LV_TXT_LINE_BREAK_LONG_POST_MIN_LEN 3

/** 支持双向文本。允许混排从左到右与从右到左的文本。
 *  方向将按 Unicode 双向算法处理：
 *  https://www.w3.org/International/articles/inline-bidi-markup/uba-basics */
#define LV_USE_BIDI 0
#if LV_USE_BIDI
    /*设置默认方向。可选值：
    *`LV_BASE_DIR_LTR` 从左到右
    *`LV_BASE_DIR_RTL` 从右到左
    *`LV_BASE_DIR_AUTO` 自动检测文本方向*/
    #define LV_BIDI_BASE_DIR_DEF LV_BASE_DIR_AUTO
#endif

/** 启用阿拉伯语/波斯语处理
 *  这些语言中，字符需根据其在文本中的位置替换为相应形态 */
#define LV_USE_ARABIC_PERSIAN_CHARS 0

/* 用于标记文本重新着色的控制字符 */
#define LV_TXT_COLOR_CMD "#"

/*==================
 * 控件
 *================*/
/* 控件文档见：https://docs.lvgl.io/master/widgets/index.html 。 */

/** 1：使以下控件在创建时被赋予默认值。
 *  - lv_buttonmatrix_t:  使用默认映射：{"Btn1", "Btn2", "Btn3", "\n", "Btn4", "Btn5", ""}，否则不设置映射。
 *  - lv_checkbox_t    :  文本标签设为 "Check box"，否则为空字符串。
 *  - lv_dropdown_t    :  选项设为 "Option 1"、"Option 2"、"Option 3"，否则不设置。
 *  - lv_roller_t      :  选项设为 "Option 1" 至 "Option 5"，否则不设置。
 *  - lv_label_t       :  文本设为 "Text"，否则为空字符串。
 *  - lv_arclabel_t   :  文本设为 "Arced Text"，否则为空字符串。
 * */
#define LV_WIDGETS_HAS_DEFAULT_VALUE  1

#define LV_USE_ANIMIMG    1

#define LV_USE_ARC        1

#define LV_USE_ARCLABEL  1

#define LV_USE_BAR        1

#define LV_USE_BUTTON        1

#define LV_USE_BUTTONMATRIX  1

#define LV_USE_CALENDAR   1
#if LV_USE_CALENDAR
    #define LV_CALENDAR_WEEK_STARTS_MONDAY 0
    #if LV_CALENDAR_WEEK_STARTS_MONDAY
        #define LV_CALENDAR_DEFAULT_DAY_NAMES {"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"}
    #else
        #define LV_CALENDAR_DEFAULT_DAY_NAMES {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"}
    #endif

    #define LV_CALENDAR_DEFAULT_MONTH_NAMES {"January", "February", "March",  "April", "May",  "June", "July", "August", "September", "October", "November", "December"}
    #define LV_USE_CALENDAR_HEADER_ARROW 1
    #define LV_USE_CALENDAR_HEADER_DROPDOWN 1
    #define LV_USE_CALENDAR_CHINESE 0
#endif  /*LV_USE_CALENDAR*/

#define LV_USE_CANVAS     1

#define LV_USE_CHART      1

#define LV_USE_CHECKBOX   1

#define LV_USE_DROPDOWN   1   /**< 依赖：lv_label */

#define LV_USE_IMAGE      1   /**< 依赖：lv_label */

#define LV_USE_IMAGEBUTTON     1

#define LV_USE_KEYBOARD   1

#define LV_USE_LABEL      1
#if LV_USE_LABEL
    #define LV_LABEL_TEXT_SELECTION 1   /**< 启用标签文本选择 */
    #define LV_LABEL_LONG_TXT_HINT 1    /**< 在标签中存储额外信息，加速超长文本绘制 */
    #define LV_LABEL_WAIT_CHAR_COUNT 3  /**< 等待字符的个数 */
#endif

#define LV_USE_LED        1

#define LV_USE_LINE       1

#define LV_USE_LIST       1

#define LV_USE_LOTTIE     1

#define LV_USE_MENU       1

#define LV_USE_MSGBOX     1

#define LV_USE_ROLLER     1   /**< 依赖：lv_label */

#define LV_USE_SCALE      1

#define LV_USE_SLIDER     1   /**< 依赖：lv_bar */

#define LV_USE_SPAN       1
#if LV_USE_SPAN
    /** 一行文本可包含的 span 描述符最大数量。 */
    #define LV_SPAN_SNIPPET_STACK_SIZE 64
#endif

#define LV_USE_SPINBOX    1

#define LV_USE_SPINNER    1

#define LV_USE_SWITCH     1

#define LV_USE_TABLE      1

#define LV_USE_TABVIEW    1

#define LV_USE_TEXTAREA   1   /**< 依赖：lv_label */
#if LV_USE_TEXTAREA != 0
    #define LV_TEXTAREA_DEF_PWD_SHOW_TIME 1500    /**< [ms] */
#endif

#define LV_USE_TILEVIEW   1

#define LV_USE_WIN        1

#define LV_USE_3DTEXTURE  0

/*==================
 * 主题
 *==================*/
/* 主题文档见：https://docs.lvgl.io/master/common-widget-features/styles/styles.html#themes 。 */

/** 简洁、出色且非常完整的主题 */
#define LV_USE_THEME_DEFAULT 1
#if LV_USE_THEME_DEFAULT
    /** 0：浅色模式；1：深色模式 */
    #define LV_THEME_DEFAULT_DARK 0

    /** 1：按下时启用放大效果 */
    #define LV_THEME_DEFAULT_GROW 1

    /** 默认过渡时间，单位毫秒。 */
    #define LV_THEME_DEFAULT_TRANSITION_TIME 80
#endif /*LV_USE_THEME_DEFAULT*/

/** 极简主题，适合作为自定义主题的起点 */
#define LV_USE_THEME_SIMPLE 1

/** 为单色显示屏设计的主题 */
#define LV_USE_THEME_MONO 1

/*==================
 * 布局
 *==================*/
/* 布局文档见：https://docs.lvgl.io/master/common-widget-features/layouts/index.html 。 */

/** 类似 CSS Flexbox 的布局。 */
#define LV_USE_FLEX 1

/** 类似 CSS Grid 的布局。 */
#define LV_USE_GRID 1

/*====================
 * 第三方库
 *====================*/
/* 库文档见：https://docs.lvgl.io/master/libs/index.html 。 */

/* 常见 API 的文件系统接口 */

/** 设置默认盘符后，文件路径可省略盘符前缀。
 *  下方驱动标识字母的使用说明见
 *  https://docs.lvgl.io/master/main-modules/fs.html#lv-fs-identifier-letters 。 */
#define LV_FS_DEFAULT_DRIVER_LETTER '\0'

/** 对接 fopen、fread 等 API。 */
#define LV_USE_FS_STDIO 1
#if LV_USE_FS_STDIO
    #define LV_FS_STDIO_LETTER 'A'
    #define LV_FS_STDIO_PATH ""         /**< 设置工作目录。文件/目录路径将附加在其后。 */
    #define LV_FS_STDIO_CACHE_SIZE 0    /**< >0 时 lv_fs_read() 缓存相应字节数 */
#endif

/** 对接 open、read 等 API。 */
#define LV_USE_FS_POSIX 0
#if LV_USE_FS_POSIX
    #define LV_FS_POSIX_LETTER '\0'     /**< 为此驱动设置大写盘符标识字母（如 'A'）。 */
    #define LV_FS_POSIX_PATH ""         /**< 设置工作目录。文件/目录路径将附加在其后。 */
    #define LV_FS_POSIX_CACHE_SIZE 0    /**< >0 时 lv_fs_read() 缓存相应字节数 */
#endif

/** 对接 CreateFile、ReadFile 等 API。 */
#define LV_USE_FS_WIN32 0
#if LV_USE_FS_WIN32
    #define LV_FS_WIN32_LETTER '\0'     /**< 为此驱动设置大写盘符标识字母（如 'A'）。 */
    #define LV_FS_WIN32_PATH ""         /**< 设置工作目录。文件/目录路径将附加在其后。 */
    #define LV_FS_WIN32_CACHE_SIZE 0    /**< >0 时 lv_fs_read() 缓存相应字节数 */
#endif

/** 对接 FATFS（需单独添加）。使用 f_open、f_read 等。 */
#define LV_USE_FS_FATFS 0
#if LV_USE_FS_FATFS
    #define LV_FS_FATFS_LETTER '\0'     /**< 为此驱动设置大写盘符标识字母（如 'A'）。 */
    #define LV_FS_FATFS_PATH ""         /**< 设置工作目录。文件/目录路径将附加在其后。 */
    #define LV_FS_FATFS_CACHE_SIZE 0    /**< >0 时 lv_fs_read() 缓存相应字节数 */
#endif

/** 对接内存映射文件访问。 */
#define LV_USE_FS_MEMFS 0
#if LV_USE_FS_MEMFS
    #define LV_FS_MEMFS_LETTER '\0'     /**< 为此驱动设置大写盘符标识字母（如 'A'）。 */
#endif

/** 对接 LittleFs。 */
#define LV_USE_FS_LITTLEFS 0
#if LV_USE_FS_LITTLEFS
    #define LV_FS_LITTLEFS_LETTER '\0'  /**< 为此驱动设置大写盘符标识字母（如 'A'）。 */
    #define LV_FS_LITTLEFS_PATH ""      /**< 设置工作目录。文件/目录路径将附加在其后。 */
#endif

/** 对接 Arduino LittleFs。 */
#define LV_USE_FS_ARDUINO_ESP_LITTLEFS 0
#if LV_USE_FS_ARDUINO_ESP_LITTLEFS
    #define LV_FS_ARDUINO_ESP_LITTLEFS_LETTER '\0'  /**< 为此驱动设置大写盘符标识字母（如 'A'）。 */
    #define LV_FS_ARDUINO_ESP_LITTLEFS_PATH ""      /**< 设置工作目录。文件/目录路径将附加在其后。 */
#endif

/** 对接 Arduino SD。 */
#define LV_USE_FS_ARDUINO_SD 0
#if LV_USE_FS_ARDUINO_SD
    #define LV_FS_ARDUINO_SD_LETTER '\0'  /**< 为此驱动设置大写盘符标识字母（如 'A'）。 */
    #define LV_FS_ARDUINO_SD_PATH ""      /**< 设置工作目录。文件/目录路径将附加在其后。 */
#endif

/** 对接 UEFI */
#define LV_USE_FS_UEFI 0
#if LV_USE_FS_UEFI
    #define LV_FS_UEFI_LETTER '\0'      /**< 为此驱动设置大写盘符标识字母（如 'A'）。 */
#endif

#define LV_USE_FS_FROGFS 0
#if LV_USE_FS_FROGFS
    #define LV_FS_FROGFS_LETTER '\0'
#endif

/** LODEPNG 解码库 */
#define LV_USE_LODEPNG 1

/** PNG 解码库（libpng） */
#define LV_USE_LIBPNG 0

/** BMP 解码库 */
#define LV_USE_BMP 1

/** JPG + 分块 JPG 解码库。
 *  Split JPG 是为嵌入式系统优化的自定义格式。 */
#define LV_USE_TJPGD 1

/** libjpeg-turbo 解码库。
 *  - 支持完整 JPEG 规范与高性能 JPEG 解码。 */
#define LV_USE_LIBJPEG_TURBO 0

/** WebP 解码库 */
#define LV_USE_LIBWEBP 0

/** GIF 解码库 */
#define LV_USE_GIF 0
#if LV_USE_GIF
    /** 加速 GIF 解码 */
    #define LV_GIF_CACHE_DECODE_DATA 0
#endif

/** GStreamer 库 */
#define LV_USE_GSTREAMER 0

/** 将 bin 图像解码到 RAM */
#define LV_BIN_DECODER_RAM_LOAD 1

/** RLE 解压库 */
#define LV_USE_RLE 1

/** 二维码库 */
#define LV_USE_QRCODE 1

/** 条形码库 */
#define LV_USE_BARCODE 1

/** FreeType 库 */
#define LV_USE_FREETYPE 0
#if LV_USE_FREETYPE
    /** 让 FreeType 使用 LVGL 的内存与文件移植层 */
    #define LV_FREETYPE_USE_LVGL_PORT 0

    /** FreeType 的字形缓存数量，即可缓存的字形个数。
     *  值越大占用内存越多。 */
    #define LV_FREETYPE_CACHE_FT_GLYPH_CNT 256
#endif

/** 内置 TTF 解码器 */
#define LV_USE_TINY_TTF 1
#if LV_USE_TINY_TTF
    /* 启用从文件加载 TTF 数据 */
    #define LV_TINY_TTF_FILE_SUPPORT 0
    #define LV_TINY_TTF_CACHE_GLYPH_CNT 128
    #define LV_TINY_TTF_CACHE_KERNING_CNT 256
#endif

/** Rlottie 库 */
#define LV_USE_RLOTTIE 0

/** 需要 `LV_USE_3DTEXTURE = 1` */
#define LV_USE_GLTF  0

/** 启用矢量图形 API
 *  需要 `LV_USE_MATRIX = 1`，
 *  以及支持矢量图形的渲染引擎，如
 *  (LV_USE_DRAW_SW 和 LV_USE_THORVG) 或 LV_USE_DRAW_VG_LITE 或 LV_USE_NEMA_VG。 */
#define LV_USE_VECTOR_GRAPHIC  1

/** 启用 src/libs 目录内自带的 ThorVG（矢量图形库）。
 *  需要 LV_USE_VECTOR_GRAPHIC */
#define LV_USE_THORVG_INTERNAL 1

/** 启用 ThorVG，假定其已安装并与项目完成链接。
 *  需要 LV_USE_VECTOR_GRAPHIC */
#define LV_USE_THORVG_EXTERNAL 0

/** 启用 NanoVG（矢量图形库） */
#define LV_USE_NANOVG 0

/** 使用 lvgl 内置 LZ4 库 */
#define LV_USE_LZ4_INTERNAL  1

/** 使用外部 LZ4 库 */
#define LV_USE_LZ4_EXTERNAL  0

/*SVG 库
 *  - 需要 `LV_USE_VECTOR_GRAPHIC = 1` */
#define LV_USE_SVG 0
#define LV_USE_SVG_ANIMATION 0
#define LV_USE_SVG_DEBUG 0

/** FFmpeg 库，用于图像解码与视频播放。
 *  支持所有主流图像格式，启用后请勿再启用其他图像解码器。 */
#define LV_USE_FFMPEG 0
#if LV_USE_FFMPEG
    /** 将输入信息转储到 stderr */
    #define LV_FFMPEG_DUMP_FORMAT 0
    /** FFmpeg 播放器控件使用 lvgl 文件路径
     *  启用后将无法打开 URL。
     *  注意：FFmpeg 图像解码器始终使用 lvgl 文件系统。 */
    #define LV_FFMPEG_PLAYER_USE_LV_FS 0
#endif

/*==================
 * 其他
 *==================*/
/* 以下部分条目的文档见：https://docs.lvgl.io/master/auxiliary-modules/index.html 。 */

/** 1：启用对象快照 API */
#define LV_USE_SNAPSHOT 0

/** 1：启用系统监视组件 */
#define LV_USE_SYSMON   1
#if LV_USE_SYSMON
    /** 获取空闲百分比。如 uint32_t my_get_idle(void); */
    #define LV_SYSMON_GET_IDLE lv_os_get_idle_percent
    /** 1：启用 lv_os_get_proc_idle_percent。*/
    #define LV_SYSMON_PROC_IDLE_AVAILABLE 0
    #if LV_SYSMON_PROC_IDLE_AVAILABLE
        /** 获取应用程序的空闲百分比。
         * - 需要 `LV_USE_OS == LV_OS_PTHREAD` */
        #define LV_SYSMON_GET_PROC_IDLE lv_os_get_proc_idle_percent
    #endif

    /** 1：显示 CPU 占用与 FPS 计数。
     *  - 需要 `LV_USE_SYSMON = 1` */
    #define LV_USE_PERF_MONITOR 0
    #if LV_USE_PERF_MONITOR
        #define LV_USE_PERF_MONITOR_POS LV_ALIGN_BOTTOM_RIGHT

        /** 0：在屏幕上显示性能数据；1：通过日志打印性能数据。 */
        #define LV_USE_PERF_MONITOR_LOG_MODE 0
    #endif

    /** 1：显示内存占用与内存碎片。
     *     - 需要 `LV_USE_STDLIB_MALLOC = LV_STDLIB_BUILTIN`
     *     - 需要 `LV_USE_SYSMON = 1`*/
    #define LV_USE_MEM_MONITOR 0
    #if LV_USE_MEM_MONITOR
        #define LV_USE_MEM_MONITOR_POS LV_ALIGN_BOTTOM_LEFT
    #endif
#endif /*LV_USE_SYSMON*/

/** 1：启用运行时性能分析器 */
#define LV_USE_PROFILER 0
#if LV_USE_PROFILER
    /** 1：启用内置性能分析器 */
    #define LV_USE_PROFILER_BUILTIN 1
    #if LV_USE_PROFILER_BUILTIN
        /** 分析器跟踪缓冲区的默认大小 */
        #define LV_PROFILER_BUILTIN_BUF_SIZE (16 * 1024)     /**< [bytes] */
        #define LV_PROFILER_BUILTIN_DEFAULT_ENABLE 1
        #define LV_USE_PROFILER_BUILTIN_POSIX 0 /**< 启用 POSIX 分析器移植 */
    #endif

    /** 性能分析器需包含的头文件 */
    #define LV_PROFILER_INCLUDE "lvgl/src/misc/lv_profiler_builtin.h"

    /** 分析器起点函数 */
    #define LV_PROFILER_BEGIN    LV_PROFILER_BUILTIN_BEGIN

    /** 分析器终点函数 */
    #define LV_PROFILER_END      LV_PROFILER_BUILTIN_END

    /** 带自定义标签的分析器起点函数 */
    #define LV_PROFILER_BEGIN_TAG LV_PROFILER_BUILTIN_BEGIN_TAG

    /** 带自定义标签的分析器终点函数 */
    #define LV_PROFILER_END_TAG   LV_PROFILER_BUILTIN_END_TAG

    /*启用布局分析器*/
    #define LV_PROFILER_LAYOUT 1

    /*启用显示刷新分析器*/
    #define LV_PROFILER_REFR 1

    /*启用绘制分析器*/
    #define LV_PROFILER_DRAW 1

    /*启用输入设备分析器*/
    #define LV_PROFILER_INDEV 1

    /*启用解码器分析器*/
    #define LV_PROFILER_DECODER 1

    /*启用字体分析器*/
    #define LV_PROFILER_FONT 1

    /*启用文件系统分析器*/
    #define LV_PROFILER_FS 1

    /*启用样式分析器*/
    #define LV_PROFILER_STYLE 0

    /*启用定时器分析器*/
    #define LV_PROFILER_TIMER 1

    /*启用缓存分析器*/
    #define LV_PROFILER_CACHE 1

    /*启用事件分析器*/
    #define LV_PROFILER_EVENT 1
#endif

/** 1：启用 Monkey 测试 */
#define LV_USE_MONKEY 0

/** 1：启用网格导航 */
#define LV_USE_GRIDNAV 0

/** 1：启用 `lv_obj` 片段（fragment）逻辑 */
#define LV_USE_FRAGMENT 0

/** 1：支持在 label 或 span 控件中用图像作为字体 */
#define LV_USE_IMGFONT 1

/** 1：启用观察者模式实现 */
#define LV_USE_OBSERVER 1

/** 1：启用拼音输入法
 *  - 依赖：lv_keyboard */
#define LV_USE_IME_PINYIN 0
#if LV_USE_IME_PINYIN
    /** 1：使用默认词库。
     *  @note 若不使用默认词库，务必在设置词库后再使用 `lv_ime_pinyin`。 */
    #define LV_IME_PINYIN_USE_DEFAULT_DICT 1
    /** 设置可显示的候选词面板最大数量。
     *  @note 需根据屏幕尺寸调整。 */
    #define LV_IME_PINYIN_CAND_TEXT_NUM 6

    /** 使用九宫格输入（k9）。 */
    #define LV_IME_PINYIN_USE_K9_MODE      1
    #if LV_IME_PINYIN_USE_K9_MODE == 1
        #define LV_IME_PINYIN_K9_CAND_TEXT_NUM 3
    #endif /*LV_IME_PINYIN_USE_K9_MODE*/
#endif

/** 1：启用文件浏览器。
 *  - 依赖：lv_table */
#define LV_USE_FILE_EXPLORER                     0
#if LV_USE_FILE_EXPLORER
    /** 路径最大长度 */
    #define LV_FILE_EXPLORER_PATH_MAX_LEN        (128)
    /** 快速访问栏，1：使用；0：不使用。
     *  - 依赖：lv_list */
    #define LV_FILE_EXPLORER_QUICK_ACCESS        1
#endif

/** 1：启用字体管理器 */
#define LV_USE_FONT_MANAGER                     0
#if LV_USE_FONT_MANAGER

/*字体管理器名称最大长度*/
#define LV_FONT_MANAGER_NAME_MAX_LEN            32

#endif

/** 启用模拟输入设备、时间模拟与截图对比。 */
#define LV_USE_TEST 0
#if LV_USE_TEST

/** 启用 `lv_test_screenshot_compare`。
 * 需要 lodepng 及额外几 MB 内存。 */
#define LV_USE_TEST_SCREENSHOT_COMPARE 0

#if LV_USE_TEST_SCREENSHOT_COMPARE
    /** 1：自动创建缺失的参考图像*/
    #define LV_TEST_SCREENSHOT_CREATE_REFERENCE_IMAGE 1
#endif /*LV_USE_TEST_SCREENSHOT_COMPARE*/

#endif /*LV_USE_TEST*/

/** 1：启用文本翻译支持 */
#define LV_USE_TRANSLATION 0

/*1：启用颜色滤镜样式*/
#define LV_USE_COLOR_FILTER     0

/*==================
 * 设备
 *==================*/

/** 在 PC 上用 SDL 打开窗口并处理鼠标与键盘。 */
#define LV_USE_SDL              1
#if LV_USE_SDL
    #define LV_SDL_INCLUDE_PATH     <SDL2/SDL.h>
    #define LV_SDL_RENDER_MODE      LV_DISPLAY_RENDER_MODE_DIRECT   /**< 推荐使用 LV_DISPLAY_RENDER_MODE_DIRECT 以获得最佳性能 */
    #define LV_SDL_BUF_COUNT        1    /**< 1 或 2 */
    #define LV_SDL_ACCELERATED      1    /**< 1：使用硬件加速*/
    #define LV_SDL_FULLSCREEN       0    /**< 1：默认全屏显示窗口 */
    #define LV_SDL_DIRECT_EXIT      1    /**< 1：所有 SDL 窗口关闭时退出应用 */
    #define LV_SDL_MOUSEWHEEL_MODE  LV_SDL_MOUSEWHEEL_MODE_ENCODER  /*LV_SDL_MOUSEWHEEL_MODE_ENCODER/CROWN*/
#endif

/** 在 Linux 桌面上用 X11 打开窗口并处理鼠标与键盘 */
#define LV_USE_X11              0
#if LV_USE_X11
    #define LV_X11_DIRECT_EXIT         1  /**< 所有 X11 窗口关闭时退出应用 */
    #define LV_X11_DOUBLE_BUFFER       1  /**< 使用双缓冲渲染 */
    /* 以下渲染模式只能选 1 个（推荐 LV_X11_RENDER_MODE_PARTIAL！）。 */
    #define LV_X11_RENDER_MODE_PARTIAL 1  /**< 局部渲染模式（推荐） */
    #define LV_X11_RENDER_MODE_DIRECT  0  /**< 直接渲染模式 */
    #define LV_X11_RENDER_MODE_FULL    0  /**< 全量渲染模式 */
#endif

/** 在 Linux 或 BSD 桌面上用 Wayland 打开窗口并处理输入 */
#define LV_USE_WAYLAND          0
#if LV_USE_WAYLAND
    #define LV_WAYLAND_DIRECT_EXIT          1     /**< 1：所有 Wayland 窗口关闭时退出应用 */
#endif

/** /dev/fb 驱动 */
#define LV_USE_LINUX_FBDEV      0
#if LV_USE_LINUX_FBDEV
    #define LV_LINUX_FBDEV_BSD           0
    #define LV_LINUX_FBDEV_RENDER_MODE   LV_DISPLAY_RENDER_MODE_PARTIAL
    #define LV_LINUX_FBDEV_BUFFER_COUNT  0
    #define LV_LINUX_FBDEV_BUFFER_SIZE   60
    #define LV_LINUX_FBDEV_MMAP          1
#endif

/** 用 Nuttx 打开窗口并处理触摸屏 */
#define LV_USE_NUTTX    0

#if LV_USE_NUTTX
    #define LV_USE_NUTTX_INDEPENDENT_IMAGE_HEAP 0

    /** 默认绘制缓冲使用独立图像堆 */
    #define LV_NUTTX_DEFAULT_DRAW_BUF_USE_INDEPENDENT_IMAGE_HEAP    0

    #define LV_USE_NUTTX_LIBUV    0

    /** 用 Nuttx 自定义初始化 API 打开窗口并处理触摸屏 */
    #define LV_USE_NUTTX_CUSTOM_INIT    0

    /** /dev/lcd 驱动 */
    #define LV_USE_NUTTX_LCD      0
    #if LV_USE_NUTTX_LCD
        #define LV_NUTTX_LCD_BUFFER_COUNT    0
        #define LV_NUTTX_LCD_BUFFER_SIZE     60
    #endif

    /** /dev/input 驱动 */
    #define LV_USE_NUTTX_TOUCHSCREEN    0

    /** 触摸屏光标尺寸，单位像素（<=0：禁用光标） */
    #define LV_NUTTX_TOUCHSCREEN_CURSOR_SIZE    0

    /** /dev/mouse 驱动 */
    #define LV_USE_NUTTX_MOUSE    0

    /** 鼠标移动步长（像素） */
    #define LV_USE_NUTTX_MOUSE_MOVE_STEP    1

    /*NuttX 跟踪文件及其路径*/
    #define LV_USE_NUTTX_TRACE_FILE 0
    #if LV_USE_NUTTX_TRACE_FILE
        #define LV_NUTTX_TRACE_FILE_PATH "/data/lvgl-trace.log"
    #endif

#endif

/** /dev/dri/card 驱动 */
#define LV_USE_LINUX_DRM        0

#if LV_USE_LINUX_DRM

    /* 使用 MESA GBM 库分配可跨子系统与库共享的 DMA 缓冲区（经由 Linux DMA-BUF API）。
     * GBM 库旨在提供平台无关的内存管理体系，
     * 支持主流 GPU 厂商——此选项需要链接 libgbm */
    #define LV_USE_LINUX_DRM_GBM_BUFFERS 0
#endif

/** TFT_eSPI 接口 */
#define LV_USE_TFT_ESPI         0

/** Lovyan_GFX 接口 */
#define LV_USE_LOVYAN_GFX         0

#if LV_USE_LOVYAN_GFX
    #define LV_LGFX_USER_INCLUDE "lv_lgfx_user.hpp"

#endif /*LV_USE_LOVYAN_GFX*/

/** evdev 输入设备驱动 */
#define LV_USE_EVDEV    0

/** libinput 输入设备驱动 */
#define LV_USE_LIBINPUT    0

#if LV_USE_LIBINPUT
    #define LV_LIBINPUT_BSD    0

    /** 完整键盘支持 */
    #define LV_LIBINPUT_XKB             0
    #if LV_LIBINPUT_XKB
        /** "setxkbmap -query" 可帮助查找键盘的正确取值 */
        #define LV_LIBINPUT_XKB_KEY_MAP { .rules = NULL, .model = "pc101", .layout = "us", .variant = NULL, .options = NULL }
    #endif
#endif

/* 通过 SPI/并口连接的 LCD 设备驱动 */
#define LV_USE_ST7735        0
#define LV_USE_ST7789        0
#define LV_USE_ST7796        0
#define LV_USE_ILI9341       0
#define LV_USE_FT81X         0
#define LV_USE_NV3007        0

#if (LV_USE_ST7735 | LV_USE_ST7789 | LV_USE_ST7796 | LV_USE_ILI9341 | LV_USE_NV3007)
    #define LV_USE_GENERIC_MIPI 1
#else
    #define LV_USE_GENERIC_MIPI 0
#endif

/** Renesas GLCD 驱动 */
#define LV_USE_RENESAS_GLCDC    0

/** ST LTDC 驱动 */
#define LV_USE_ST_LTDC    0
#if LV_USE_ST_LTDC
    /* 仅用于局部渲染。 */
    #define LV_ST_LTDC_USE_DMA2D_FLUSH 0
#endif

/** NXP ELCDIF 驱动 */
#define LV_USE_NXP_ELCDIF   0

/** LVGL Windows 后端 */
#define LV_USE_WINDOWS    0

/** LVGL UEFI 后端 */
#define LV_USE_UEFI 0
#if LV_USE_UEFI
    #define LV_USE_UEFI_INCLUDE "myefi.h"   /**< 隐藏实际框架（EDK2、gnu-efi 等）的头文件 */
    #define LV_UEFI_USE_MEMORY_SERVICES 0   /**< 使用启动服务表中的内存函数 */
#endif

/** 使用通用 OpenGL 驱动，可嵌入其他应用，或配合 GLFW/EGL 使用
 * - 需要 LV_USE_MATRIX。
 */
#define LV_USE_OPENGLES   0
#if LV_USE_OPENGLES
    #define LV_USE_OPENGLES_DEBUG        1    /**< 启用/禁用 opengles 调试 */
#endif

/** 在 PC 上用 GLFW 打开窗口并处理鼠标与键盘。需要*/
#define LV_USE_GLFW   0


/** QNX Screen 显示与输入驱动 */
#define LV_USE_QNX              0
#if LV_USE_QNX
    #define LV_QNX_BUF_COUNT        1    /**< 1 或 2 */
#endif

/** 启用/禁用外部数据与析构函数 */
#define LV_USE_EXT_DATA   0

/*=====================
* 构建选项
*======================*/

/** 将示例随库一同构建。 */
#define LV_BUILD_EXAMPLES 1

/** 构建 demo */
#define LV_BUILD_DEMOS 1

/*===================
 * 演示用法
 ====================*/

#if LV_BUILD_DEMOS
    /** 展示一些控件。可能需要增大 `LV_MEM_SIZE`。 */
    #define LV_USE_DEMO_WIDGETS 1

    /** 演示编码器与键盘的用法。 */
    #define LV_USE_DEMO_KEYPAD_AND_ENCODER 1

    /** 对系统进行基准测试 */
    #define LV_USE_DEMO_BENCHMARK 1

    #if LV_USE_DEMO_BENCHMARK
        /** 使用位图按 16 字节对齐、行跨度为 16 字节整数倍的字体 */
        #define LV_DEMO_BENCHMARK_ALIGNED_FONTS 0
    #endif

    /** 对每个图元的渲染测试。
     *  - 至少需要 480x272 显示屏。 */
    #define LV_USE_DEMO_RENDER 1

    /** LVGL 压力测试 */
    #define LV_USE_DEMO_STRESS 1

    /** 音乐播放器演示 */
    #define LV_USE_DEMO_MUSIC 1
    #if LV_USE_DEMO_MUSIC
        #define LV_DEMO_MUSIC_SQUARE    0
        #define LV_DEMO_MUSIC_LANDSCAPE 0
        #define LV_DEMO_MUSIC_ROUND     0
        #define LV_DEMO_MUSIC_LARGE     0
        #define LV_DEMO_MUSIC_AUTO_PLAY 0
    #endif

    /** 矢量图形演示 */
    #define LV_USE_DEMO_VECTOR_GRAPHIC  0

    /** GLTF 演示 */
    #define LV_USE_DEMO_GLTF            0

    /*---------------------------
     * 来自 lvgl/lv_demos 的演示
      ---------------------------*/

    /** Flex 布局演示 */
    #define LV_USE_DEMO_FLEX_LAYOUT     1

    /** 类智能手机的多语言演示 */
    #define LV_USE_DEMO_MULTILANG       1

    /*带 Lottie 动画的电踏车演示（需启用 LV_USE_LOTTIE）*/
    #define LV_USE_DEMO_EBIKE           0
    #if LV_USE_DEMO_EBIKE
        #define LV_DEMO_EBIKE_PORTRAIT  0    /*0：适用于 480x270..480x320，1：适用于 480x800..720x1280*/
    #endif

    /** 高分辨率演示 */
    #define LV_USE_DEMO_HIGH_RES        0

    /* 智能手表演示 */
    #define LV_USE_DEMO_SMARTWATCH      0
#endif /* LV_BUILD_DEMOS */

/*--END OF LV_CONF_H--*/

#endif /*LV_CONF_H*/

#endif /*"启用配置内容"结束*/
