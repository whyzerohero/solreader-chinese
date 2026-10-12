# solreader-cli：没有发现新的 First Edition 编程入口

日期：2026-10-12。对象：First Edition、用户固件v1.6.0、Windows 10。
本轮仅云端源码审计、ZIP二进制解析和离线模型；没有打开串口、发送PROGRAM、
调用esptool/OpenOCD、复位、擦写、修改驱动/eFuse或重复完整固件构建。
PR #2保持Open；阅读引擎扩展继续暂停。

## 结论与用户已提供的实机证据

**solreader-cli没有修复USB切换的设备端实现，也没有提供新的ROM/救援入口。
它能简化取得有效下载串口之后的整包刷写，不能解决当前尚无真实1001枚举的阻断。**

用户已完成此前docs/13建议的PnP观察：303A:4002的MI_00、MI_02均Present=True、Status=OK；
303A:1001全部Present=False，而且历史节点来自其他ESP32-S3，不是眼镜。
CONNECT曾成功，PROGRAM曾发送，OpenOCD找不到JTAG，随后COM6曾出现WinError31；眼镜目前正常。
这是用户提供的事实，本轮未重新采集。当前健康4002快照不能反推失败时的切换时间线。
旧1001记录不能作为本机曾进入编程的证据；不能建议选择该历史COM或再次盲发PROGRAM。
WinError31不自动等于设备管理器ProblemCode31；不能认定换驱动必然解决。

## 固定来源与完整刷机调用链

- 第三方：[collinarnett/solreader-cli](https://github.com/collinarnett/solreader-cli/tree/db769460455e05ea54b1460b86493ecf4f9785eb)，SHA `db769460455e05ea54b1460b86493ecf4f9785eb`。
- 官方v1.6.0：embedded-app `7dcf0850daa5cec076c8d68e2f56cf413ff707aa`；IDF v5.3.2/TinyUSB1.5.0沿用[docs/13](13_v1.6.0刷机可行性审计.md)证据。
- CLI历史包含官方desktop-app研究提交`650a93af3472d868543fc2e792ea722f02b061d5`；`e42258a87284950ca71d18cbb4b7d005a1fefd65`删除旧Python桌面代码，随后新增Haskell CLI。历史Python Windows支持/read_flash不等于当前CLI功能。
- 已完整阅读Main.flash/runSession、Device.program/sendLine、Serial.withPort/sendBytes、Ports和Firmware全部函数、命令表、构建清单/锁文件，并检索全部src/app入口。哈希及ZIP解析见[sources/official/solreader_cli_audit.json](../sources/official/solreader_cli_audit.json)。

【源码确认】[Main.hs:235](https://github.com/collinarnett/solreader-cli/blob/db769460455e05ea54b1460b86493ecf4f9785eb/app/Main.hs#L235)：

1. 有`--jtag-port`直接采用字符串，不检查存在、VID/PID、目标身份或协议；不发送PROGRAM。
2. 否则先findJtagPort，找到就直接刷写；没有眼镜身份核验。
3. 未找到才withDevice port program：采用`--port`或等待5秒发现控制口，打开115200串口。runSession不先CONNECT，program只发送`PROGRAM\n`，不等ACK。
4. Serial.withPort通过bracket关闭串口，再以200ms轮询等待15秒findJtagPort；超时退出，不调用flashFirmware。
5. 有端口后才读ZIP、解压、检查embedded-app.bin/flash.sh。**无效或不存在的ZIP也可能在检查失败前已经发送PROGRAM；不是安全dry-run。**
6. 读取应用0x30处32字节显示版本，没有版本/硬件/签名兼容性判定。
7. flashArgs按Haskell words分词，丢第一个词，给bin/hex路径加目录，前插`-p PORT`，用typed-process直接运行`esptool`。不经shell执行flash.sh，但保留复位及写入参数；不是shell解析器，不保证引号/空格、注释、多行脚本正确。
8. 只判断esptool退出状态，无全Flash备份、失败回退、恢复通路或刷后健康检查。

证据：[Device.hs:109/:242](https://github.com/collinarnett/solreader-cli/blob/db769460455e05ea54b1460b86493ecf4f9785eb/src/Sol/Device.hs#L109)、
[Serial.hs:28](https://github.com/collinarnett/solreader-cli/blob/db769460455e05ea54b1460b86493ecf4f9785eb/src/Sol/Serial.hs#L28)、
[Firmware.hs:35/:71](https://github.com/collinarnett/solreader-cli/blob/db769460455e05ea54b1460b86493ecf4f9785eb/src/Sol/Firmware.hs#L35)。
README/Device注释称PROGRAM为reboot；**固定官方源码实际仅切换运行时PHY，不调用esp_restart或直接进入ROM**。
宿主包装不会改变当前眼镜的USBSerial.initJtag/end；不完整TinyUSB退栈和未检查PHY返回值仍见docs/13，不能确定实机唯一根因。

## 与官方工具的关键差异

| 项目 | 官方v1.6.0 port_tool.py /已审计桌面工具 | 当前solreader-cli |
| --- | --- | --- |
| 进入模式 | 控制口发`\nPROGRAM\n`，flush/close，无ACK | `PROGRAM\n`后关闭，无额外设备端修复，也不先CONNECT |
| 等待 | port_tool最多3秒；官方桌面流程已有15秒 | 15秒，只改善迟到且可发现的端口；本机是否迟到未知 |
| Windows发现 | WMI匹配4002/串号/MI；pyserial COM找303A:1001 | **无COM/WMI/SetupAPI发现**；显式覆盖只是手工路径候选，未验证Win10运行 |
| Linux发现 | pyserial描述匹配USB JTAG/serial debug unit | /dev/serial/by-id名称含JTAG_serial_debug_unit，排序选首个存在路径 |
| macOS发现 | pyserial列表匹配描述 | control固定/dev/cu.usbmodemsolreader1；JTAG没有macOS自动发现分支 |
| 覆盖端口 | 底层esptool已支持指定已有端口 | --jtag-port跳过发现/PROGRAM，不创建编程接口 |
| 烧录协议 | esptool串行ROM；另有OpenOCD JTAG路径 | esptool串行ROM，**不用OpenOCD/JTAG调试协议** |
| 驱动 | OpenOCD需JTAG接口能被libusb访问；esptool需串口 | 要可打开的串口；参数名JTAG不意味着要求/修复WinUSB调试驱动 |
| 整包参数 | 官方Release四目标写入 | 复用flash.sh的default_reset、write_flash、hard_reset，无备份闭环 |
| 预检副作用 | 旧官方desktop verify先擦otadata | 无该verify调用；flash仍真实复位/擦写，无只读安全预检 |

【源码确认】[Ports.hs:17–61](https://github.com/collinarnett/solreader-cli/blob/db769460455e05ea54b1460b86493ecf4f9785eb/src/Sol/Ports.hs#L17)
不读取USB VID/PID、不枚举JTAG PnP接口；名称匹配可选中其他Espressif板。
flake.nix只列x86_64-linux，没有当前Windows构建/CI验证，cabal未约束serialport版本。
不能证明手工构建一定不能在Windows运行，但**不能宣称支持/修复Windows 10**。
solreader.cabal license字段为空，未找到LICENSE；第三方源码/固件不重新分发。

【源码确认】PROGRAM与ROM复位是两步。现有工具链esptool **4.12.0**的loader.py:713/:738在PID1001或usb_reset时
选择USBJTAGSerialReset；reset.py:124以DTR/RTS进入下载，另有Windows控制位传播处理。
这在**已有可打开的USJ串口之后**，不是在4002上修复TinyUSB→USJ切换。
这是已安装版本的静态对照，不代表CLI锁定Nix构建实际使用的esptool版本；本轮没运行这些函数。
指定COM6不会把正常应用CDC变成ROM。如果本机已有USJ串口，esptool可以不依赖OpenOCD调试驱动，
但**目前没有该眼镜Present1001，不能据此安排主动串口复位实验**。

## 附带固件：历史包，不是v1.6.0恢复备份

【离线验证】仅一个ZIP：`embedded-app-b3db33b2461bdb98dd0062852cc14527c2142949-IDFv5.1.4.zip`，
2,059,830字节，SHA-256 `14c2df908082b83a860f0cc02aa1c97edc1c06554796684a07642cbe0e8bb0e8`。
CRC/路径通过；__MACOSX/._*是AppleDouble元数据，不是固件。
真实ESP32-S3镜像chip_id=9、app描述magic=0xabcd5432：

- version=`release/v1.0.6-8-gb3db33b2`，project=embedded-app，IDF=v5.1.4。
- 编译Aug 14 2024 / 17:45:43，secure_version=0；不证明设备安全策略允许降级。
- 应用3,180,688字节，SHA-256 `b551325df38f41a9dc509d45ae2a318f4dc57837b994e84cba33a2f953611356`。
- 官方Git对象b3db33b2461bdb98dd0062852cc14527c2142949存在；旧initJtag同样拆CDC/卸PHY并忽略usb_new_phy结果。不能通过降级来验证切换故障修复。

ZIP只有bootloader.bin、partition-table.bin、ota_data_initial.bin、embedded-app.bin、flash.sh（另有元数据）。
script指定460800、default_reset/hard_reset、esp32s3、dio/64MB/80m；四写入位置0x0、0x8000、0xe000、0x10000。
写入会擦目标区域扇区，覆写引导/分区/启动选择；不是只读，不等于一定全片擦除。

| 分区 | 偏移 | 长度 | 包内内容 |
| --- | --- | --- | --- |
| nvs | 0x9000 | 0x5000 | 无设备数据 |
| otadata | 0xe000 | 0x2000 | 初始OTA数据，覆写启动选择 |
| app0 | 0x10000 | 0x400000 | 历史应用 |
| app1 | 0x410000 | 0x400000 | 无镜像 |
| coredump | 0x810000 | 0x10000 | 无镜像 |
| data | 0x820000 | 0x37e0000 | 无书籍/设置 |

分区表哈希与官方v1.6.0候选包相同，bootloader/应用不同。布局一致不证明生产配置、签名、加密、防回滚兼容。
没有独立factory app槽；此包不是本机NVS/校准/配对/双槽/书籍或全片备份，无法证明可靠恢复。
CLI的restore只恢复readables.json书架记录，会传文件/重启；library snapshot也不是完整书籍或Flash备份。

## A–E独立判断与备份先决条件

| 层次 | 当前状态 |
| --- | --- |
| A PROGRAM切换USB | 【源码确认】有设计，CLI仍发同一命令，实际成功未知 |
| B Windows正确枚举本机1001 | 【待实机验证】未确认；现有1001是其他硬件历史 |
| C USB-JTAG调试驱动绑定 | 【待实机验证】无本机Present1001；4002健康不证明JTAG绑定，串行ROM另需可用串口 |
| D ROM下载协议连接 | 【阻断】无有效端口/握手记录，参数覆盖不证明权限 |
| E 原厂Flash备份和恢复 | 【阻断】CLI无完整备份，无应用坏后独立救援/恢复实证 |

沿用docs/13另五项：官方刷写源码入口成立；实机编程、原厂读回、安全自定义写入、可靠恢复均未证实。
备份需稳定且允许读取的ROM/JTAG通路、实际容量/分区/活动槽和安全策略证据；
两次全量读回哈希相符、私有保存设备数据；区分密文/明文、地址绑定及签名限制。
写坏应用后不能再依赖它处理PROGRAM，必须预先有独立重入办法；候选发布包不代替本机备份/恢复实证。

## 下一步只保留一个方向，停止这条工具路线

**现在没有值得直接在唯一眼镜上执行的新方法。唯一建议：取得厂家明确的First Edition v1.6.0
不依赖运行应用的下载/救援进入与退出流程，以及匹配恢复包。**
助手不主动联系外部人员；所需问题记录为：不拆机/板级入口、USB控制器路径、量产下载/eFuse策略、
失败退出方式、型号/版本对应镜像和数据保留范围。若只有拆机服务入口，应如实接受限制。

现在不提供新PROGRAM/flash/reset步骤，也不重复已完成PnP查询。可靠入口/退出证据到位后，
再单独授权并设计一次最小ROM握手/只读识别：明确身份、预期枚举/协议响应、时间上限及恢复措施，
不直接write_flash。无1001、目标不明、WinError31、阅读异常、要求换驱动或任何擦写/eFuse操作立即停止。
当前尚无经验证主动切换的恢复措施，因此主动实机实验前提未满足。
**换宿主包装、延长等待或覆盖历史COM未解决入口阻断，停止沿同流程反复尝试。**

## 实际离线校验与限制

```sh
python3 tools/recovery/audit_cli_offline.py \
  --cli /workspace/work/solreader-cli-audit \
  --report sources/official/solreader_cli_audit.json
python3 tools/build/verify_project.py
git diff --check
```

新增16项源码守卫、Python发现/流程模型、ZIP CRC/哈希、应用头/版本、刷写参数和分区解析，实际16通过/0失败。
覆盖参数跳过PROGRAM、无身份核验、4秒/16秒到达、Windows/macOS发现缺口。
**模型是人工翻译的行为检查，不是Haskell执行；没有GHC/runghc，未安装新工具或编译CLI。**
不模拟Windows驱动、USB电气/竞态、ROM协议或eFuse，不宣称实机刷写/恢复成功。
此前官方16项故障注入结果保留，本轮未重复无关研究或阅读器测试。

项目结构/20条规则/固定引用/隐私忽略、Python AST/JSON、README名称和diff检查通过。
曾尝试verify_project.py --source-root /workspace/work/official，但完整检出校验因tiny-epub目录缺失而失败；
当前只保留本任务需要的embedded-app和desktop-app，两个检出的HEAD/干净状态/直接子模块树另行核验。
未为此重新下载无关仓库，不能把不带source-root的结构检查宣称为全部六个检出校验成功。
