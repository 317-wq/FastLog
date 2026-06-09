/**
 * 示例 05：日志文件切分
 *
 * RotatingFileSink 按文件大小自动切分：
 *   app.log      — 当前文件
 *   app.log.1    — 上一个切分文件
 *   app.log.2    — 更早的切分文件
 *   ...
 *
 * 当日志文件超过设定大小，自动重命名并创建新文件。
 */

#include <filesystem>
#include <fstream>
#include <iostream>

#include "../include/sink/rotating_file_sink.h"

int main() {
    const std::string path = "logs/05_rotate.log";

    // 最大 500 字节，最多保留 3 个旧文件
    ljt::RotatingFileSink sink(path, 500, 3);

    std::cout << "写入 200 条日志（每条 ~25 字节），预计触发多次切分..." << std::endl;

    for (int i = 0; i < 200; ++i) {
        sink.log(ljt::Level::INFO,
                 "rotate msg #" + std::to_string(i) + " padding");
    }

    sink.flush();

    std::cout << "\n生成的文件：" << std::endl;
    // 检查哪些文件存在
    for (int i = 0; i <= 5; ++i) {
        std::string p = i == 0 ? path : path + "." + std::to_string(i);
        std::ifstream ifs(p);
        if (ifs.good()) {
            int lines = 0;
            std::string dummy;
            while (std::getline(ifs, dummy)) ++lines;
            auto sz = std::filesystem::file_size(p);
            std::cout << "  " << p << "  (" << lines << " 行, "
                      << sz << " bytes)" << std::endl;
        }
    }

    return 0;
}
