/**
 * 示例 04：彩色控制台输出
 *
 * 使用 ColorStdoutSink 在终端输出带颜色的日志。
 * 不同等级显示不同颜色：TRACE白 DEBUG青 INFO绿 WARN黄 ERROR红 CRITICAL红底白字
 */

#include <iostream>

#include "../include/logger.h"
#include "../include/macros.h"
#include "../include/sink/color_stdout_sink.h"

int main() {
    auto sink = std::make_shared<ljt::ColorStdoutSink>();
    ljt::Logger logger("color", sink);

    std::cout << "彩色终端输出效果：\n" << std::endl;

    LOG_TRACE(logger,   "TRACE — 白色：最详细的调试信息");
    LOG_DEBUG(logger,   "DEBUG — 青色：调试信息");
    LOG_INFO(logger,    "INFO  — 绿色：一般信息");
    LOG_WARN(logger,    "WARN  — 黄色：警告");
    LOG_ERROR(logger,   "ERROR — 红色：错误");
    LOG_CRITICAL(logger,"CRITICAL — 红底白字：严重错误");

    std::cout << "\n以上各等级在终端中会显示不同颜色。文件输出不会带颜色。"
              << std::endl;
    return 0;
}
