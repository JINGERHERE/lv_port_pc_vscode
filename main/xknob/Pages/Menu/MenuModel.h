#ifndef __MENU_MODEL_H
#define __MENU_MODEL_H

/* [移植改动] 本文件由 X-Knob 的 MenuModel.h 移植而来。
 * 原工程依赖 AccountSystem（消息框架）与 HAL：
 * 原: #include "app/Accounts/Account_Master.h"
 *
 * MenuModel 原本订阅了 "Motor" 主题，但全工程无任何地方调用 ChangeMotorMode()，
 * 也就是说本页 UI 并不消费电机数据。PC 模拟器放弃 AccountSystem / 电机 HAL，
 * 故 MenuModel 退化为空壳，仅保留 Init/Deinit 两个生命周期接口。
 */

namespace Page
{

class MenuModel
{
public:
    void Init();
    void Deinit();

    /* [移植改动] 以下接口依赖已停用的 AccountSystem / HAL，暂不启用。
     * 待将来移植 AccountSystem + 假 HAL 时再放开。
     * 原: void GetIMUInfo(char* info, uint32_t len);
     * 原: void GetBatteryInfo(int* usage, float* voltage, char* state, uint32_t len);
     * 原: void ChangeMotorMode(int mode);
     */

private:
    /* [移植改动] 原为指向 AccountSystem 节点的成员，依赖已停用。
     * 原: Account* account;
     */

private:
};

}

#endif
