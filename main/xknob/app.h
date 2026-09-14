#ifndef __APP_H__
#define __APP_H__

/* [移植改动] 本文件由 X-Knob 的 app/app.h 移植而来。
 * 原文件依赖 Arduino / ESP32 任务通知 / 电机 HAL：
 * 原: #include "hal/motor.h"
 * 现将这些依赖以注释形式保留（不删除），只保留 PC 模拟器需要的枚举与宏。
 */

/* [移植改动] 原为 AccountSystem 的通知宏，依赖 Account_Master.h（未移植）。
 * #define ACCOUNT_SEND_NOTIFY_CMD(ACT, CMD)\
 * do{\
 *     AccountSystem::ACT##_Info_t info;\
 *     memset(&info, 0, sizeof(info));\
 *     info.cmd = AccountSystem::CMD;\
 *     AccountSystem::Broker()->AccountMaster.Notify(#ACT, &info, sizeof(info));\
 * }while(0)
 */

/* [移植改动] 原 Arduino 分支依赖 port/display.h 与 FreeRTOS 任务通知（LCD 任务初始化完成信号）。
 *            非 Arduino 分支本就是一个空宏，PC 模拟器沿用空宏即可。
 * 原: #ifdef ARDUINO
 * 原: #include "port/display.h"
 * 原: #define INIT_DONE() do{ xTaskNotifyGive(handleTaskLvgl); }while(0)
 * 原: #else
 * 原: #define INIT_DONE() do{ }while(0)
 * 原: #endif
 */
#define INIT_DONE() do{ }while(0)

/* [移植改动] 上游 app.h 依赖 <Arduino.h> 间接提供标准 offsetof，
 *            因而下面的 `#ifndef offsetof` 分支在 ESP32 上从不展开。
 *            PC 端去掉了 Arduino 依赖，必须自备标准 offsetof，否则会去展开那个分支。
 *            注意该分支上游本身有缺陷（跨行却漏了行尾反斜杠），已一并修正。
 */
#include <stddef.h>

#ifndef offsetof
    #define offsetof(type, member) ((size_t)&reinterpret_cast<char const volatile&>      \
          ((((type*)0)->member)))
#endif

#ifndef container_of
#define container_of(ptr, type, member) ({          								\
     const typeof( ((type *)0)->member ) *__mptr = (ptr);    	\
     (type *)( (char *)__mptr - offsetof(type,member) );})
#endif

/* [移植改动] 原声明依赖 ESP32 显示初始化（port/display.h）。PC 端显示由 main.c 的
 *            hal_init() 通过 SDL 完成，故停用该声明。
 * 原: void display_init();
 */

void App_Init();
void App_UnInit();


enum PLAYGROUND_MODE {
    PLAYGROUND_MODE_NO_EFFECTS,
    PLAYGROUND_MODE_FINE_DETENTS,
    PLAYGROUND_MODE_BOUND ,
    PLAYGROUND_MODE_ON_OFF,
    PLAYGROUND_MODE_MAX,
};

enum APP_MODE {
    APP_MODE_SUPER_DIAL = PLAYGROUND_MODE_MAX,
    APP_MODE_HOME_ASSISTANT,
    APP_MODE_SETTING,
    APP_MODE_MAX,
};

enum SETTING_MODE {
    SETTING_MODE_LCD_BK_BRIGHTNESS = APP_MODE_MAX,
    SETTING_MODE_LCD_BK_TIMEOUT,
    SETTING_MODE_MAX,
};

#endif /* __APP_H__ */
