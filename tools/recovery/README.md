# 备份与恢复：尚未验证

官方 Release 包不是原厂设备备份，OTA 双槽不是恢复成功证明。
未知：实机 Flash/eFuse/加密状态、实际分区、可用 ROM/JTAG 通路、恢复所需驱动与板级资料。

后续只读备份阶段需记录读取方式和版本、完整范围、重复读取 SHA-256、
分区和 NVS/校准/配对数据、存储加密状态、恢复依赖；敏感内容只能私有保存。
上游 BootLoopDetection 存在先清数据再回滚的路径及可疑分区读取判断，
在离线修正验证前不能作为可信恢复机制。不得运行 FactoryReset 或上游 unpair-remote。

新增audit_usb_offline.py：对固定官方源函数做临时编译和桩注入，16项检查实际通过；
不导入serial/esptool，不打开真实端口。结论及最小只读观察实验见docs/13。
桌面esptool_verify并非只读：源码会先擦otadata，不执行该真实工具。

solreader-cli完整源码调查见docs/14；docs/13的PnP方案用户已完成，不重复要求。
audit_cli_offline.py只做固定第三方源码守卫、Python行为模型和ZIP解析，16项通过；
不执行/编译Haskell、不打开端口、不导入serial/esptool或执行ZIP脚本。
第三方--jtag-port不创造下载接口，附带2024年旧包不是本机v1.6.0恢复备份。
