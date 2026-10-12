# 云端离线工具

fetch_official.py：按 manifest 克隆研究源码，固定 SHA，不递归子模块、不执行上游代码。
verify_project.py：检查必需文件、已删除的错误依赖/事件耦合、锁定 SHA、忽略规则，以及指定检出中的 HEAD 和子模块记录。
audit_fonts.py：只读取 IBMF Unicode 字符目录及 TTF cmap，输出汉字覆盖报告；不执行字体或上游脚本。
工具都使用 Python 标准库；不安装 ESP-IDF，不提供固件 build/flash 自动化。

v1.6.0 构建意图为 ESP-IDF v5.3.2、esp32s3、glasses-release，
但完整 sdkconfig、工具链包、组件锁定及编译尚未完成。第三方脚本本轮仅审计。
