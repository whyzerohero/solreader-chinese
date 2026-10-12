# SolReader-Chinese

Chinese-first minimalist reading and writing firmware for Sol Reader, with future dedicated one-handed Chinese input hardware.

面向 **Sol Reader First Edition** 的长期固件项目，以中文阅读和中文写作为核心。
利用可合法复用的官方硬件底层、驱动与工具，开发自己的极简中文阅读固件。
用户此前自制极简阅读固件的阅读交互、分页、排版和功能设计是上层产品设计的重要参考；
未经源码、许可和平台兼容性检查，不宣称已有固件可直接移植、编译或刷写。

目前完成的是工程初始化与官方源码初步审计，**尚无自定义固件、实机测试或安全刷机工具**。

## 产品路线

- 第一阶段：简体中文 TXT/EPUB、中文字体、UTF-8、标点禁则、中英混排、字号、跳转、选书和阅读位置恢复。
- 第二阶段：未来独立单手实体输入硬件驱动的九宫格中文输入、候选词、编辑与笔记；本轮只规划接口。
- 第三阶段：可复现构建、可验证原厂备份、Windows 10 更新工具、回滚和恢复；须经过真实验证。

原厂遥控器只有五个实体按键（用户已确认），用于翻页和菜单等基本阅读控制，不用于九宫格输入。
专用输入硬件的按键数量、布局、连接协议和通信方式尚未确定；接口保持独立且可扩展。

不人为删掉官方成熟 EPUB、Unicode 或英文支持。GBK/GB18030 优先考虑电脑导入时转换，后续根据实际资源决定设备端支持。

## 版本与当前阻断

用户实机返回 `VERSION SEMVER,1.6.0;GIT,release/v1.6.0`。
官方 Tag `release/v1.6.0` 指向 `7dcf0850daa5cec076c8d68e2f56cf413ff707aa`，
候选 First Edition 构建配置为 ESP32-S3 / ESP-IDF v5.3.2 / glasses-release。
官方发布包 SHA-256 已离线核对，不能据此认定与实机逐字节一致。

五个主要官方仓库未找到顶层许可证，授权范围待澄清；源码以固定 SHA 清单管理，不复制进公开 Git 历史。
v1.6.0 的阅读和字体代码仍在 embedded-app 内；之后 main 才拆分 tiny-epub、tiny-font、cpp-misc，不能混装。
附带字体不覆盖常用汉字；中文换行与禁则需要补充。遥控器固件、服务端和硬件设计未找到可访问资料。
原厂回滚逻辑有可疑读取判断，并在回滚前执行数据清除，见恢复审计。

## 工程入口

| 位置 | 用途 |
| --- | --- |
| [AGENTS.md](AGENTS.md) | 长期规则及设备安全约束 |
| [docs/10_开发进度.md](docs/10_开发进度.md) | 结果、阻断与下一任务 |
| [docs/02_官方源码清单.md](docs/02_官方源码清单.md) | 24 个公开仓库清单、许可证、锁定引用 |
| sources/official/ | 固定 SHA、上游子模块与公开发布包校验记录 |
| [docs/08_自制极简阅读固件设计参考.md](docs/08_自制极简阅读固件设计参考.md) | 上层产品设计参考及源码审计边界 |
| firmware/solreader_custom/ | 独立接口契约和模块设计，无硬件驱动实现 |
| tools/build/ | 安全的来源核验、字体覆盖检查 |
| tools/flash/、tools/recovery/ | 未验证的更新与恢复条件，无刷写脚本 |
| tests/ | 本机接口编译检查及未来中文测试矩阵 |
| backups/、outputs/ | 私有备份与生成物目录，不进入公开 Git |

## 云端继续工作

清单版本固定在 [manifest.lock.json](sources/official/manifest.lock.json)。
可使用本项目的 `python3 tools/build/fetch_official.py` 取得这些只读研究引用；
它只克隆固定提交，不初始化传递子模块、不运行上游脚本、不安装工具链、不接触设备。
首次只检出 embedded-app 等研究源码，缺失的完整工具链和传递依赖不能称为可复现构建。

```sh
python3 tools/build/verify_project.py
python3 tools/build/verify_project.py --source-root /workspace/work/official
c++ -std=c++17 -Wall -Wextra -Werror -pedantic -fsyntax-only tests/contract_compile.cpp
python3 tools/build/audit_fonts.py --source-root /workspace/work/official
```

以上都是离线检查。不要运行上游 make 的 flash、unpair-remote、program-mode、
port_tool.py 的 programming/reset_jtag 或 monitor；这些可切换 USB、复位或写入设备。
不连接 Windows 电脑或眼镜，不执行 PROGRAM、OpenOCD、ROM 下载或任何擦写操作。
开发分支经 PR 持久化，不自动合并 main。
