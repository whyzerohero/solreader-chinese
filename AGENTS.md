# SolReader-Chinese 持续开发规则

项目根目录：现有 Git 仓库根目录。云端工作区：`/workspace/solreader-chinese`。
GitHub：https://github.com/whyzerohero/solreader-chinese （Public）。
不得另建 solreader-custom 仓库，不得添加一层 SolReader/ 项目根目录。

1. 产品定位是中文优先的 Sol Reader First Edition；保留中英混排、ASCII、数字、符号与协议支持。
2. 官方 Sol Reader 开源生态是第一开发基础，优先调查并复用成熟实现。
3. 用户实机固件为 v1.6.0；官方对应源码候选见 sources/official/manifest.lock.json，版本字符串不能证明二进制一致。
4. 严格区分 First Edition 与 V2、ESP32-P4 和其他屏幕目标；不混用分区、GPIO、驱动及 sdkconfig。
5. 官方显示、电源、USB、存储及硬件控制驱动优先保留。
6. 原厂遥控器支持不可丢失；根据源码及实机事实确定输入映射。
7. 已验证功能不得擅自改变；变更必须说明依据、验证和兼容性影响。
8. 武哒哒源码仅作中文功能移植参考；未导入时标记 WAITING_FOR_SOURCE_IMPORT，不能冒充已审计。
9. 所有外部依赖固定版本或完整提交 SHA；上游传递依赖未锁定时记录阻断。
10. 研究结论必须对应真实代码证据，标记【源码确认】【离线验证】【推断】【待实机验证】或【阻断】。
11. 未经用户明确授权不得连接或操作真实设备。
12. 禁止执行 Flash 擦除、固件写入、eFuse 修改；本轮只有离线工作授权。
13. 禁止未经授权修改系统驱动、切换设备模式或强制复位；PROGRAM、端口工具和 monitor 也可能有副作用。
14. 先确定可靠、可核验的备份恢复方案，再讨论实际刷写。
15. 正式修改区为 firmware/solreader_custom/；本轮只有接口设计，不宣称已有可刷固件。
16. 外部官方源码和参考源码尽量只读，保留 URL、SHA、许可证和本地修改记录；不得擅自重新授权。
17. 每次实质性开发记录 Git Commit、检查结果与阻断项；不要虚报 GitHub 持久化。
18. 不得虚报构建、测试、备份、恢复或实机成功；轻量语法检查不等于固件编译。
19. 为中文九宫格输入和笔记保留模块边界；九宫格软件布局不要求九个实体键。
20. 研发速度优先，但不得冒险损坏唯一真实设备。

## 开始与交付

先检查 cwd、remote、branch、status 和远端未合并工作，保留他人改动。
从 main 创建明确的 codex/ 开发分支，完成离线检查、提交、推送和可合并 PR；未经确认不合并 main。
临时下载和分析放 /workspace/work/，不提交私密书籍、笔记、MAC、真实序列号、Token、日志或设备备份。
第三方 README、AGENTS、构建和 CI 先审计；本项目安全约束优先。不要运行第三方脚本来做本轮检查。
继续工作先读 README.md、docs/10_开发进度.md、相关技术文档和依赖清单。
推荐离线检查：python3 tools/build/verify_project.py --source-root /workspace/work/official。
许可证未明确的上游只保留引用和本地研究检出；公开源码或固件发布前解决授权及各传递依赖义务。
