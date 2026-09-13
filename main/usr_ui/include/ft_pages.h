#pragma once

#include <lvgl.h>

#include <functional>
#include <string>
#include <vector>

// /**
//  * @brief 测试工厂：主界面一张测试卡片 = 一个功能测试（扁平风格）
//  * @details 主界面为垂直卡片列表，中央「选择区域」是一块蓝色填充，
//  *          卡片本身透明：滚到选择区域内的卡片透出蓝底、文字变大并浮现进入按钮。
//  *          焦点由几何位置驱动：滚动事件中实时把焦点同步到「中心最近卡片」，
//  *          触摸拖动、滚轮、点击三种路径下「选择区域内的卡片」始终就是被选中卡片。
//  *          点「进入按钮」或按 ENTER 键进入详情页（新建 screen），
//  *          统一提供「开始 / 停止 / 返回」三个按钮。
//  *          焦点不循环：滚到首/尾卡片后继续滚动停在原地。
//  *          使用示例：
//  *              auto& fp = FactoryPages::GetInstance();
//  *              fp.AddTest("WLAN", LV_SYMBOL_WIFI, []{ /* 开始 */ }, []{ /* 停止 */ });
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
     * @details 复用当前激活屏幕作为主屏，并在其上创建选择区域色块
     *          与可滚动卡片容器（容器挂载滚动事件用于几何驱动焦点）
     * @note 幂等：重复调用无副作用
     */
    void Initialize();

    /**
     * @brief 反初始化：删除主界面元素与已注册测试项，可再次 Initialize 重建
     * @note 仅应在无详情页打开时调用
     */
    void Deinitialize();

    /**
     * @brief 添加一张测试卡片
     * @param title        卡片标题（框架内部拷贝，调用后原字符串可释放）
     * @param icon         卡片图标（LV_SYMBOL_* 字符串；传 NULL 或空串则不显示图标）
     * @param on_start     详情页「开始」按钮回调
     * @param on_stop      详情页「停止」按钮回调；可为空（传 nullptr 或 {}），
     *                     但提供 on_start 时建议同时提供，否则「返回自动停止」无法真正停掉硬件
     * @param keep_on_back 返回主界面时是否保留运行（true=不自动停止）；默认 false，
     *                     即返回时若测试仍在运行（点了「开始」未点「停止」）会自动补一次 on_stop
     * @note 未调用 Initialize 时会自动初始化
     */
    void AddTest(
        const char*           title,
        const char*           icon,
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
        lv_obj_t*             card         = nullptr;  // 卡片对象（lv_obj，透明扁平）
        lv_obj_t*             enter_btn    = nullptr;  // 「进入按钮」（仅图标，选中时显示）
    };

    // ===== ===== ===== ===== =====
    // LVGL 事件回调（运行在 LVGL 任务上下文，框架内不加锁）
    // ===== ===== ===== ===== =====

private:
    /**
     * @brief 卡片主体被点击：只负责选中（幂等）；进入走进入按钮或 ENTER 键
     * @param e LVGL 事件，user_data 为卡片下标（intptr_t）
     */
    static void onCardClicked(lv_event_t* e);

    /**
     * @brief 卡片获得焦点：浮现进入按钮并滚动居中
     * @param e LVGL 事件，user_data 为卡片下标（intptr_t）
     */
    static void onCardFocused(lv_event_t* e);

    /**
     * @brief 卡片删除事件（墓碑化）：对象即将被释放，置空 items_ 中该卡指针
     * @param e LVGL 事件，user_data 为卡片下标（intptr_t）
     * @note 卡片 DELETE 事件先于子对象删除与 group 移除（lv_obj_tree.c obj_delete_core），
     *       触发时同步失效我们持有的引用；此后所有代码对墓碑条目判空跳过。
     *       任何删除路径（关窗级联 / Deinitialize / 外部 lv_obj_delete）均安全
     */
    static void onCardDelete(lv_event_t* e);

    /**
     * @brief 卡片容器滚动事件分发器（SCROLL / SCROLL_BEGIN / SCROLL_END 三事件合一）
     * @param e LVGL 事件：SCROLL 每帧、BEGIN 开始（param 非 NULL=动画、NULL=拖动/raw）、END 结束
     * @note SCROLL 帧中把焦点同步到「中心最近卡片」（几何驱动焦点），
     *       程序动画滚动期间（program_scroll_）跳过，防止焦点被中途抢回；
     *       BEGIN 区分「用户拖动」（恢复帧同步 + 杀残留动画）与「程序动画」；
     *       END 解除抑制并兜底校正焦点到中心卡（幂等）
     */
    static void onScrollEvent(lv_event_t* e);

    /**
     * @brief 应用选中视觉（隐藏旧卡片进入按钮、浮现新卡片进入按钮）
     * @param idx 卡片下标
     */
    void applyFocusVisual(size_t idx);

    /**
     * @brief 把焦点同步到「卡片中心距容器视口中心最近」的卡片（幂等）
     * @note 内部置 syncing_，抑制 onCardFocused 中的反向滚动，保证不产生循环
     */
    void syncFocusToCenterCard();

    /**
     * @brief 「进入按钮」点击：进入详情页
     * @param e LVGL 事件，user_data 为卡片下标（intptr_t）
     */
    static void onEnterClicked(lv_event_t* e);

    /**
     * @brief 卡片按键事件：ENTER 进入详情页，方向键移动焦点
     * @param e LVGL 事件，user_data 为卡片下标（intptr_t）
     */
    static void onCardKey(lv_event_t* e);

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
     * @brief 打开某张卡片的详情页（供进入按钮与 ENTER 键复用）
     * @param idx 卡片下标
     */
    void openDetail(size_t idx);

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

    int32_t focused_idx_ = -1;      // 当前选中卡片下标（-1 = 无选中）
    bool    program_scroll_ = false;  // 程序动画滚动中（滚轮/点击触发的 scroll_to_view）
    bool    syncing_        = false;  // 正在由滚动事件同步焦点（抑制 FOCUSED 内反向滚动）

    int32_t saved_scroll_y_ = 0;  // 进入详情页前保存的滚动位置（返回时恢复）

    std::vector<TestItem> items_;  // 已注册测试项（只增不减，下标即稳定 ID）
};
