# 正式开发区

当前：ARCHITECTURE_ONLY。include/solreader/contracts.hpp 是原创、设备无关的接口提案，
没有固件入口、ESP-IDF 工程、屏幕驱动或完整输入法，不能生成可刷固件。

模块：
- input：原厂 RemoteCommand 到统一 InputEvent，保留可用的原始通知与来源；在 UI 队列路由。
- text：UTF-8 校验、中文断行与标点禁则；GBK/GB18030 转换首先放在导入工具。
- fonts：明确授权的汉字资源、按字号读取与有上限的缓存；保留 ASCII 和官方 UI 字符。
- reading：适配官方流、TXT/EPUB、分页和阅读位置，复用原厂屏幕、字号与跳转。
- ime：有限按钮导航软件九宫格、候选词与组合状态，供未来实现；未指定实体键映射。
- notes：UTF-8 编辑、撤回、原子保存、模式切换；独立于书籍和原厂 NVS 数据。
- platform：未来只包装原厂显示、存储、电源、USB 与调度生命周期，不重写 GPIO/分区。

优先基于 v1.6.0 内嵌引擎验证中文缺口；后续独立 tiny-* 只是研究参考。
正式固件整合前必须澄清上游授权、核验 First Edition 配置、完成离线中文验证；
实机写入另需明确授权及可信备份/恢复证据。
