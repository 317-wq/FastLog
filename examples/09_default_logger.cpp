/**
 * 示例 09：默认全局 Logger — 零配置快速使用
 *
 * FLOG_* 系列宏无需创建 Logger，开箱即用。
 * 默认输出到 stdout（带颜色），适合快速原型和调试。
 */

#include <iostream>

#include "../include/default_logger.h"

int main() {
    std::cout << "默认 Logger 零配置使用（彩色输出到终端）：\n" << std::endl;

    FLOG_TRACE("这是一条 TRACE 日志");
    FLOG_DEBUG("这是一条 DEBUG 日志");
    FLOG_INFO("这是一条 INFO 日志");
    FLOG_WARN("这是一条 WARN 日志");
    FLOG_ERROR("这是一条 ERROR 日志");
    FLOG_CRITICAL("这是一条 CRITICAL 日志");

    std::cout << "\n也可以设置过滤等级：" << std::endl;

    ljt::setDefaultLevel(ljt::Level::WARN);
    std::cout << "设置等级为 WARN 后：" << std::endl;

    FLOG_INFO("这条 INFO 不会显示（INFO < WARN）");
    FLOG_WARN("这条 WARN 会显示");
    FLOG_ERROR("这条 ERROR 也会显示");

    // 恢复默认
    ljt::setDefaultLevel(ljt::Level::TRACE);

    return 0;
}
