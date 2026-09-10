#pragma once

#include <lvgl.h>

#include <functional>
#include <string>
#include <vector>

// /**
//  * @brief 测试工厂：主界面一张测试卡片 = 一个功能测试
//  * @details 点卡片进入详情页（新建 screen），统一提供「开始 / 停止 / 返回」三个按钮。
//  *          使用示例：
//  *              auto& fp = FactoryPages::GetInstance();
//  *              fp.AddTest("WLAN", []{ /* 开始 */ }, []{ /* 停止 */ });
//  */
class FactoryPages {
public:
    /**
     * @brief 获取单例实例
     * @return FactoryPages& 单例引用
     */
    static FactoryPages& GetInstance() {
        static FactoryPages instance;  // 单例，永不析构
        return instance;
    }

    // 禁止拷贝 / 赋值
    FactoryPages(const FactoryPages&)            = delete;
    FactoryPages& operator=(const FactoryPages&) = delete;

public:
    /**
     * @brief 初始化主界面
     * @details 复用当前激活屏幕作为主屏，并在其上创建可滚动卡片容器
     * @note 幂等：重复调用无副作用
     */
    void Initialize();

    /**
     * @brief 反初始化：删除卡片容器与已注册测试项，可再次 Initialize 重建
     * @note 仅应在无详情页打开时调用
     */
    void Deinitialize();

    /**
     * @brief 添加一张测试卡片
     * @param title        卡片标题（框架内部拷贝，调用后原字符串可释放）
     * @param on_start     详情页「开始」按钮回调
     * @param on_stop      详情页「停止」按钮回调；可为空（传 nullptr 或 {}），
     *                     但提供 on_start 时建议同时提供，否则「返回自动停止」无法真正停掉硬件
     * @param keep_on_back 返回主界面时是否保留运行（true=不自动停止）；默认 false，
     *                     即返回时若测试仍在运行（点了「开始」未点「停止」）会自动补一次 on_stop
     * @note 未调用 Initialize 时会自动初始化
     */
    void AddTest(
        const char*           title,
        std::function<void()> on_start,
        std::function<void()> on_stop,
        bool                  keep_on_back = false
    );

private:
    // 单例构造 / 析构
    FactoryPages()  = default;
    ~FactoryPages() = default;

    /**
     * @brief 单张测试卡片的数据
     */
    struct TestItem {
        std::string           title;                 // 卡片标题（详情页标题复用）
        std::function<void()> on_start;              // 「开始」按钮回调
        std::function<void()> on_stop;               // 「停止」按钮回调（可为空）
        bool                  keep_on_back = false;  // 返回时是否保留运行（true=不自动停止）
        bool                  is_running   = false;  // 当前是否正在运行
    };

    // ===== ===== ===== ===== =====
    // LVGL 事件回调（运行在 LVGL 任务上下文，框架内不加锁）
    // ===== ===== ===== ===== =====

private:
    /**
     * @brief 卡片点击：创建详情页并切换过去
     * @param e LVGL 事件，user_data 为卡片下标（intptr_t）
     */
    static void onCardClicked(lv_event_t* e);

    /**
     * @brief 详情页「开始」按钮点击
     * @param e LVGL 事件，user_data 为卡片下标
     */
    static void onStart(lv_event_t* e);

    /**
     * @brief 详情页「停止」按钮点击
     * @param e LVGL 事件，user_data 为卡片下标
     */
    static void onStop(lv_event_t* e);

    /**
     * @brief 详情页「返回」按钮点击：切回主屏并删除详情页
     * @param e LVGL 事件，user_data 为卡片下标（intptr_t）
     */
    static void onBack(lv_event_t* e);

    /**
     * @brief 把所有输入设备切到指定焦点组
     * @param g 目标焦点组（group）
     */
    static void setAllIndevGroup(lv_group_t* g);

    // ===== ===== ===== ===== =====
    // 内部状态
    // ===== ===== ===== ===== =====
private:
    bool initialized_ = false;  // 是否已初始化

    lv_obj_t* main_scr_  = nullptr;  // 主屏指针
    lv_obj_t* card_cont_ = nullptr;  // 卡片容器（可滚动 flex 列）

    lv_group_t* main_group_   = nullptr;  // 主界面焦点组（卡片）
    lv_group_t* detail_group_ = nullptr;  // 详情页焦点组（开始/停止/返回按钮）

    int32_t saved_scroll_y_ = 0;  // 进入详情页前保存的滚动位置（返回时恢复）

    lv_style_t focus_style_;  // 焦点样式（橙色底色 + 向下投影）

    std::vector<TestItem> items_;  // 已注册测试项（只增不减，下标即稳定 ID）
};
