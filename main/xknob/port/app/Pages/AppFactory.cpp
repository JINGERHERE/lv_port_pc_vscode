/*
 * MIT License
 * Copyright (c) 2021 _VIFEXTech
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "AppFactory.h"
#include "_Template/Template.h"
// [移植改动] 以下页面依赖 AccountSystem / HAL（电机 / WiFi / MQTT / SD 卡），
//            PC 模拟器暂未移植，故停用。后续按阶段逐个放开。
// 原: #include "SystemInfos/SystemInfos.h"
// 原: #include "StartUp/StartUp.h"
#include "Menu/Menu.h"
// 原: #include "Playground/Playground.h"
// 原: #include "HASS/Hass.h"
// 原: #include "SurfaceDial/SurfaceDial.h"
// 原: #include "Setting/Setting.h"

// #include "Scene3D/Scene3D.h"

// [移植改动] 原工程靠 <Arduino.h> 间接引入 strcmp，PC 端需显式包含
#include <string.h>

#define APP_CLASS_MATCH(className)\
do{\
    if (strcmp(name, #className) == 0)\
    {\
        return new Page::className;\
    }\
}while(0)

PageBase* AppFactory::CreatePage(const char* name)
{
    APP_CLASS_MATCH(Template);
    APP_CLASS_MATCH(Menu);
    // [移植改动] 以下页面暂未移植（依赖 AccountSystem / HAL），先注释保留
    // APP_CLASS_MATCH(Playground);
    // APP_CLASS_MATCH(SurfaceDial);
    // APP_CLASS_MATCH(Startup);
    // APP_CLASS_MATCH(Hass);
    // APP_CLASS_MATCH(Setting);
    // APP_CLASS_MATCH(Scene3D);

    return nullptr;
}
