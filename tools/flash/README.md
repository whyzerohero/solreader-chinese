# 固件更新工具：尚未实现

本轮不提供可执行刷写、PROGRAM、端口切换或驱动安装脚本。
官方 port_tool.py 的 find_port programming 会发送 PROGRAM；find_port logging 会发 LOGGING_ON；
reset_jtag 会改变 DTR/RTS，因此不能将这些命令当作只读端口枚举。
Windows COM 编号必须动态发现，不能固定 COM6/COM7。

未来 Windows 10 工具先完成明确设备识别、授权范围、只读备份、校验、
原厂包来源和配置、恢复路径、更新中断处理、回滚离线验证，再申请实机阶段授权。
参考 docs/07_刷机备份与恢复.md；目前不具备发布或实机更新条件。
