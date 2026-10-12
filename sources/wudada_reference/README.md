# wudada FIX6 内部自制阅读技术参考

当前基线为已收到并审计的 UI_POLISH_FIX6 私有源码参考包，不再是“源码暂未导入”。
实际 ZIP SHA256：183619586db19e1bcff54cb3c5640428047b34c4deee1be4e4306364099e05cd；109文件。
内部代号指用户自制阅读技术，公开项目为 SolReader-Chinese，公开 README 不展示此入口/代号。

原文件在 imported/FIX6/wudada_154_reader_UI_POLISH_FIX6/；未修改，测试后109文件哈希一致。
私有适配在 imported/FIX6/adapted/；公开 fix6_audit.lock.json 仅记录选用的原版/适配输入哈希，不包含实现。
详细功能、复用、差异、测试和许可限制见 [FIX6审计](../../docs/12_FIX6源码审计与适配.md)。

优先复用流式分页、导航及编码/进度算法；不复用旧 GPIO、屏幕、分区、电源、睡眠或 firmware.bin。
已通过原版 reader/navigation/encoding/progress 四套测试及20个原创适配测试，仍未完成设备固件移植。
ZIP、原/适配源码和字库仅供私下研究，均在 Git 忽略目录；未经确认不得公开提交。
不要提交参考源码包附带的私人样本或第三方字体；内部代码命名不表示公开分发授权。
