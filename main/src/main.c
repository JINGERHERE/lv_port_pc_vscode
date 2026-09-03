
/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#define _DEFAULT_SOURCE /* needed for usleep() */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lv_demos_ext.h"
#include "lvgl/demos/lv_demos.h"
#include "lvgl/examples/lv_examples.h"
#include "lvgl/lvgl.h"

#if !defined(_WIN32)
    #include "glob.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_display_t* hal_init(int32_t w, int32_t h);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

extern void freertos_main(void);

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      VARIABLES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**********************
 *   USER INCLUDE
 **********************/
#include "test_ui.h"

#if !defined(_WIN32)
int main(int argc, char** argv)
#else
int SDL_main(int argc, char** argv)
#endif
{
    (void)argc; /*Unused*/
    (void)argv; /*Unused*/

    /*Initialize LVGL*/
    lv_init();

    /*Initialize the HAL (display, input devices, tick) for LVGL*/
    hal_init(512, 512);

#if LV_USE_OS == LV_OS_NONE

    /*---------------------------
     * 用户 UI
    ---------------------------*/
    // lv_demo_widgets();
    // lv_demo_benchmark();

    /*---------------------------
     * Demos from lvgl/lv_demos
    ---------------------------*/
    // lv_demo_smartwatch();
    lv_demo_flex_layout();

    /*---------------------------
     * 用户 UI
    ---------------------------*/
    //   my_gui();
    //   round_gui();
    // img_gui();

    /*---------------------------
     * LVGL 主循环
    ---------------------------*/
    while (1) {
        /* Periodically call the lv_task handler.
         * It could be done in a timer interrupt or an OS task too.*/
        lv_timer_handler();
        usleep(5 * 1000);
    }

#elif LV_USE_OS == LV_OS_FREERTOS

    /* Run FreeRTOS and create lvgl task */
    freertos_main();

#endif

    return 0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the LVGL graphics
 * library
 */
static lv_display_t* hal_init(int32_t w, int32_t h) {
    lv_group_set_default(lv_group_create());

    lv_display_t* disp = lv_sdl_window_create(w, h);

    lv_indev_t* mouse = lv_sdl_mouse_create();
    lv_indev_set_group(mouse, lv_group_get_default());
    lv_indev_set_display(mouse, disp);
    lv_display_set_default(disp);

    LV_IMAGE_DECLARE(mouse_cursor_icon); /*Declare the image file.*/
    lv_obj_t* cursor_obj;
    cursor_obj = lv_image_create(lv_screen_active()); /*Create an image object for the cursor */
    lv_image_set_src(cursor_obj, &mouse_cursor_icon); /*Set the image source*/
    lv_indev_set_cursor(mouse, cursor_obj);           /*Connect the image  object to the driver*/

    lv_indev_t* mousewheel = lv_sdl_mousewheel_create();
    lv_indev_set_display(mousewheel, disp);
    lv_indev_set_group(mousewheel, lv_group_get_default());

    lv_indev_t* kb = lv_sdl_keyboard_create();
    lv_indev_set_display(kb, disp);
    lv_indev_set_group(kb, lv_group_get_default());

    return disp;
}