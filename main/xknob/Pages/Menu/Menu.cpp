#include "Menu.h"
#include "Configs/Version.h"
// [移植改动] 电机 HAL 依赖 ESP32 + SimpleFOC + MT6701 磁编码器，PC 模拟器不启用。
//            本页仅在 onSystemEvent 里用 HAL::power_off()，该调用已同步注释。
// 原: #include "hal/motor.h"
#include "app.h"

/* [移植改动] 说明：lv_get_indev() 由 Menu.h -> MenuView.h -> ../Page.h -> lv_obj_ext_func.h 提供，
 *            无需额外包含（移植时已把其内部的 indev->driver->type 改为 v9 的 lv_indev_get_type()）。
 */
/* [移植改动] 原工程靠 <Arduino.h> 间接引入 printf，PC 端需显式包含 <stdio.h> */
#include <stdio.h>
using namespace Page;

Menu::Menu()
{
}

Menu::~Menu()
{

}

void Menu::onCustomAttrConfig()
{

}

void Menu::onViewLoad()
{
	Model.Init();
	View.Create(_root);
	AttachEvent(_root, onPlaygroundEvent);
	AttachEvent(View.ui.dialpad.icon, onSuperDialEvent);
	AttachEvent(View.ui.switches.icon, onPlaygroundEvent);
	AttachEvent(View.ui.hass.icon, onHassEvent);
	AttachEvent(View.ui.system.icon, onSystemEvent);
	AttachEvent(View.ui.setting.icon, onSettingEvent);
	// AttachEvent(View.ui.imu.icon);
	// AttachEvent(View.ui.battery.icon);
	// AttachEvent(View.ui.storage.icon);
}

void Menu::onViewDidLoad()
{

}

void Menu::onViewWillAppear()
{
	lv_indev_set_group(lv_get_indev(LV_INDEV_TYPE_ENCODER), View.ui.group);
	// StatusBar::SetStyle(StatusBar::STYLE_BLACK);

	timer = lv_timer_create(onTimerUpdate, 100, this);
	lv_timer_ready(timer);

	View.SetScrollToY(_root, -LV_VER_RES, LV_ANIM_OFF);
	lv_obj_fade_in(_root, 300, 0);
}

void Menu::onViewDidAppear()
{
	View.onFocus(View.ui.group);
}

void Menu::onViewWillDisappear()
{
	lv_obj_fade_out(_root, 300, 0);
}

void Menu::onViewDidDisappear()
{
	lv_timer_del(timer);
}

void Menu::onViewDidUnload()
{
	View.Delete();
	Model.Deinit();
}

void Menu::AttachEvent(lv_obj_t* obj, lv_event_cb_t event_cb)
{
	lv_obj_set_user_data(obj, this);
	lv_obj_add_event_cb(obj, event_cb, LV_EVENT_ALL, this);
}

void Menu::Update()
{
	char buf[64];

	/* System */
	View.SetSystem(
		VERSION_FIRMWARE_NAME " " VERSION_SOFTWARE,
		VERSION_AUTHOR_NAME,
		VERSION_LVGL,
		"dummy time",
		VERSION_COMPILER,
		VERSION_BUILD_TIME
	);
}

void Menu::onTimerUpdate(lv_timer_t* timer)
{
	// [移植改动] LVGL v9 的 lv_timer_t 已不透明，user_data 字段不可直接访问
	// 原: Menu* instance = (Menu*)timer->user_data;
	Menu* instance = (Menu*)lv_timer_get_user_data(timer);

	instance->Update();
}

void Menu::onPlaygroundEvent(lv_event_t* event)
{
	// [移植改动] LVGL v9 的 lv_event_get_target() 返回 void*，C++ 下需用 lv_event_get_target_obj()
	// 原: lv_obj_t* obj = lv_event_get_target(event);
	lv_obj_t* obj = lv_event_get_target_obj(event);
	lv_event_code_t code = lv_event_get_code(event);
	auto* instance = (Menu*)lv_obj_get_user_data(obj);

	if (code == LV_EVENT_PRESSED) {
		// instance->Model.ChangeMotorMode(MOTOR_FINE_DETENTS);
		instance->_Manager->Push("Pages/Playground");
	}
}


void Menu::onSystemEvent(lv_event_t* event)
{
	// [移植改动] LVGL v9 的 lv_event_get_target() 返回 void*，C++ 下需用 lv_event_get_target_obj()
	// 原: lv_obj_t* obj = lv_event_get_target(event);
	lv_obj_t* obj = lv_event_get_target_obj(event);
	lv_event_code_t code = lv_event_get_code(event);
	auto* instance = (Menu*)lv_obj_get_user_data(obj);
	if (code == LV_EVENT_PRESSED) {
		printf("Power off...\n");
		// [移植改动] Arduino 的 Serial.printf 在 PC 上不可用，改用 printf
		// 原: Serial.printf("Power off...\n");
		// [移植改动] 电源管理依赖 ESP32 硬件，PC 模拟器不启用
		// 原: HAL::power_off();
	}
}

void Menu::onSettingEvent(lv_event_t* event)
{
	// [移植改动] LVGL v9 的 lv_event_get_target() 返回 void*，C++ 下需用 lv_event_get_target_obj()
	// 原: lv_obj_t* obj = lv_event_get_target(event);
	lv_obj_t* obj = lv_event_get_target_obj(event);
	lv_event_code_t code = lv_event_get_code(event);
	auto* instance = (Menu*)lv_obj_get_user_data(obj);
	if (code == LV_EVENT_PRESSED) {
		printf("Menu: onSystemEvent LV_EVENT_PRESSED\n");
		instance->_Manager->Push("Pages/Setting");
	}
}


void Menu::onSuperDialEvent(lv_event_t* event)
{
	// [移植改动] LVGL v9 的 lv_event_get_target() 返回 void*，C++ 下需用 lv_event_get_target_obj()
	// 原: lv_obj_t* obj = lv_event_get_target(event);
	lv_obj_t* obj = lv_event_get_target_obj(event);
	lv_event_code_t code = lv_event_get_code(event);
	auto* instance = (Menu*)lv_obj_get_user_data(obj);

	if (code == LV_EVENT_PRESSED) {
		// instance->Model.ChangeMotorMode(MOTOR_FINE_DETENTS);
//		int16_t mode = APP_MODE_SUPER_DIAL;
//		Stash_t stash;
//		stash.ptr = &mode;
//		stash.size = sizeof(int16_t);
//		instance->_Manager->Push("Pages/SurfaceDial", &stash);
		instance->_Manager->Push("Pages/SurfaceDial");
	}
}

void Menu::onHassEvent(lv_event_t* event)
{
	// [移植改动] LVGL v9 的 lv_event_get_target() 返回 void*，C++ 下需用 lv_event_get_target_obj()
	// 原: lv_obj_t* obj = lv_event_get_target(event);
	lv_obj_t* obj = lv_event_get_target_obj(event);
	lv_event_code_t code = lv_event_get_code(event);
	auto* instance = (Menu*)lv_obj_get_user_data(obj);

	if (code == LV_EVENT_SHORT_CLICKED) {
		printf("Menu: onHassEvent LV_EVENT_PRESSED\n");
		instance->_Manager->Push("Pages/Hass");
	}
}
