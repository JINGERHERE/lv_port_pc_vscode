#include "MenuModel.h"

/* [移植改动] 本文件由 X-Knob 的 MenuModel.cpp 移植而来。
 * 原工程的依赖与实现全部依赖 AccountSystem / Arduino / HAL，PC 模拟器不启用，
 * 因此原实现整段以注释形式保留（不删除），只保留可编译的空壳实现。
 *
 * 原: #include <stdio.h>
 * 原: #include <Arduino.h>
 * 原: using namespace Page;
 */

using namespace Page;

/* ===== [移植改动] 以下为原实现，依赖已停用的 AccountSystem / Serial =====
 *
 * void MenuModel::Init()
 * {
 *     account = new Account("MenuModel", AccountSystem::Broker(), 0, this);
 *     account->Subscribe("Motor");
 *     // account->Subscribe("IMU");
 *     // account->Subscribe("Power");
 *     // account->Subscribe("Storage");
 * }
 *
 * void MenuModel::Deinit()
 * {
 *     if (account)
 *     {
 *         delete account;
 *         account = nullptr;
 *     }
 * }
 *
 * void MenuModel::ChangeMotorMode(int mode)
 * {
 *     Serial.printf("MenuModel: Change Motor Mode\n");
 *     AccountSystem::Motor_Info_t info;
 *     info.cmd = AccountSystem::MOTOR_CMD_CHANGE_MODE;
 *     info.motor_mode = mode;
 *     // 第一个参数是通知发布者，即本 Account 应该 subscribe 第一个参数指向的 Account
 *     account->Notify("Motor", &info, sizeof(info));
 * }
 *
 * （原文件中还被注释掉的两个实现，一并保留如下）
 * // void MenuModel::GetBatteryInfo(
 * //     int* usage,
 * //     float* voltage,
 * //     char* state, uint32_t len
 * // )
 * // {
 * //     HAL::Power_Info_t power;
 * //     account->Pull("Power", &power, sizeof(power));
 * //     *usage = power.usage;
 * //     *voltage = power.voltage / 1000.0f;
 * //     strncpy(state, power.isCharging ? "CHARGE" : "DISCHARGE", len);
 * // }
 *
 * // void MenuModel::GetStorageInfo(
 * //     bool* detect,
 * //     char* usage, uint32_t len
 * // )
 * // {
 * //     AccountSystem::Storage_Basic_Info_t info;
 * //     account->Pull("Storage", &info, sizeof(info));
 * //     *detect = info.isDetect;
 * //     snprintf(
 * //         usage, len,
 * //         "%0.1f GB",
 * //         info.totalSizeMB / 1024.0f
 * //     );
 * // }
 * ========================================================================= */

void MenuModel::Init()
{
    /* PC 模拟器：无 AccountSystem / 电机 HAL，无需初始化任何东西 */
}

void MenuModel::Deinit()
{
    /* 同上，无资源需要释放 */
}
