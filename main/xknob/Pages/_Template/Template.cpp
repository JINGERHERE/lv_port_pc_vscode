#include "Template.h"
// [移植改动] Arduino 头文件在 PC 模拟器上不可用，且本文件未使用任何 Arduino API
// 原: #include <Arduino.h>
using namespace Page;

Template::Template()
{
}

Template::~Template()
{

}

void Template::onCustomAttrConfig()
{
	SetCustomCacheEnable(true);
	SetCustomLoadAnimType(PageManager::LOAD_ANIM_OVER_BOTTOM, 500, lv_anim_path_bounce);
}

void Template::onViewLoad()
{
	View.Create(_root);
	lv_label_set_text(View.ui.labelTitle, _Name);

	AttachEvent(_root);
	AttachEvent(View.ui.canvas);

	Model.TickSave = Model.GetData();
}

void Template::onViewDidLoad()
{

}

void Template::onViewWillAppear()
{
	Param_t param;
	param.color = lv_color_white();
	param.time = 100;

	PAGE_STASH_POP(param);

	lv_obj_set_style_bg_color(_root, param.color, LV_PART_MAIN);

	// timer = lv_timer_create(onTimerUpdate, param.time, this);
}

void Template::onViewDidAppear()
{

}

void Template::onViewWillDisappear()
{

}

void Template::onViewDidDisappear()
{
	// lv_timer_del(timer);
}

void Template::onViewDidUnload()
{

}

void Template::AttachEvent(lv_obj_t* obj)
{
	lv_obj_set_user_data(obj, this);
	lv_obj_add_event_cb(obj, onEvent, LV_EVENT_ALL, this);
}

void Template::Update()
{
	lv_label_set_text_fmt(View.ui.labelTick, "tick = %d save = %d", Model.GetData(), Model.TickSave);
}

void Template::onTimerUpdate(lv_timer_t* timer)
{
	// [移植改动] LVGL v9 的 lv_timer_t 已不透明，user_data 字段不可直接访问
	// 原: Template* instance = (Template*)timer->user_data;
	Template* instance = (Template*)lv_timer_get_user_data(timer);

	instance->Update();
}

void Template::onEvent(lv_event_t* event)
{
	// [移植改动] LVGL v9 的 lv_event_get_target() 返回 void*，C++ 下需用 lv_event_get_target_obj()
	// 原: lv_obj_t* obj = lv_event_get_target(event);
	lv_obj_t* obj = lv_event_get_target_obj(event);
	lv_event_code_t code = lv_event_get_code(event);
	auto* instance = (Template*)lv_obj_get_user_data(obj);

	if (code == LV_EVENT_PRESSED)
	{
		instance->_Manager->Push("Pages/Menu");
	}
}
