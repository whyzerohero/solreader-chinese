# 正式开发区

当前：已有设备无关 TXT 主机原型，尚未接入设备固件。include/solreader/contracts.hpp 保持独立输入语义接口，
没有固件入口、ESP-IDF 工程、屏幕驱动或完整输入法，不能生成可刷固件。

`src/txt_reader.cpp` 实现严格 UTF-8、原文范围映射、中文/混排分页、导航、重排及内容绑定进度记录。
`tools/build/test_reader.sh` 在主机编译运行 26 个测试；`SANITIZE=1` 开启 ASan/UBSan。
当前优先通过reference_reader.hpp/cpp链接真实FIX6私有算法适配，独立原型保留作比较；它不是完整固件移植证明。

原型目前完整持有文本、码点、行和页索引，内存随文本增长，尚不能称为 ESP32-S3 长书阅读实现。
禁则采用实测 advance 的严格宽度规则：能放下的标点留在本行；放不下时下一行 `prohibited_start` 明确标记降级，
不删标点、不回退搬移前一个汉字；超宽单字标记 `oversized_glyph`，渲染后端须裁剪。
BOM/CR/LF/CRLF 保留在 `source_begin..source_end`，显示范围不含 BOM 和换行；分页区间可重建原文。
FNV-1a 仅用于意外内容变更/记录损坏检测，不是安全认证。进度文件采用原子 rename；未验证断电持久性或官方 NVS/LittleFS。
抽象绘制使用行顶坐标，真实后端须处理 baseline、字形 ink 边界、裁剪以及与测量一致性；尚未接入官方显示。

模块：
- reading_input：原厂五键遥控器 RemoteCommand 到 InputEvent，保留原始通知与来源，仅路由翻页、菜单等阅读控制。
- dedicated_input：未来独立单手实体输入硬件到 CompositionEvent 的独立可扩展适配；按键数量、布局、连接协议和通信方式未定。
- text：UTF-8 校验、中文断行与标点禁则；GBK/GB18030 转换首先放在导入工具。
- fonts：明确授权的汉字资源、按字号读取与有上限的缓存；保留 ASCII 和官方 UI 字符。
- reading：自研极简中文阅读上层；用户自制固件的交互、分页、排版与功能设计是重要参考。官方流和位置格式是研究与可选兼容对象，复用须审计许可和适用性。
- ime：独立专用硬件的九宫格中文输入、候选词与组合状态，只接收 CompositionEvent；不接受原厂 InputEvent，不采用五键逐格导航方案。
- notes：UTF-8 编辑、撤回、原子保存、模式切换；独立于书籍和原厂 NVS 数据。
- platform：未来只包装原厂显示、存储、电源、USB 与调度生命周期，不重写 GPIO/分区。

利用可合法复用的官方硬件底层、驱动与工具开发自己的固件；优先复用内部wudada算法，不直接复制旧硬件固件。
优先对照 v1.6.0 研究结果验证中文缺口；后续独立 tiny-* 只是研究参考。
正式固件整合前必须澄清上游授权、核验 First Edition 配置、完成离线中文验证；
实机写入另需明确授权及可信备份/恢复证据。

## 真实参考后端

`reference_reader.cpp` 是原创平台 facade，实际链接私有FIX6的编码/分页/协作导航/进度核心。
`reading_controller.hpp` 把原厂逻辑事件接到上一页/下一页/菜单；不接入CompositionEvent。
私有实现不进入公开仓库，编译前按sources/wudada_reference/fix6_audit.lock.json核对输入。

```sh
SANITIZE=1 tools/build/test_reference.sh sources/wudada_reference/imported/FIX6/adapted
```

20个适配测试通过；真实中文字体主机绘制和目标单元编译记录见docs/12。
私有后端使用有界缓存/32页历史，构造时两次顺序扫描验证UTF-8和指纹，不持有整本书。
源必须不可变并存活。restore/reflow可能启动协作导航；busy期间不保存或刷新，step完成后再显示。
原厂NVS/LittleFS、官方波形/字形渲染尚未集成。私有断行保留空白/换行；
未照搬旧硬件squeeze，宽度不足的标点起行明确标记降级。
