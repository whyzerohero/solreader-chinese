# 固定官方来源

manifest.lock.json 是依赖机制：记录固定 Git 提交、候选标签、子模块原始 URL 和 SHA。
ecosystem_inventory.json 是 2026-10-12 的公开仓库发现快照，其 default_head 不是 First Edition 兼容版本。
release_v1.6.0.json 记录已离线校验的官方发布包；它不是用户设备备份。

五个核心仓库未找到顶层许可证，不能推定有重新授权或任意再发布权限。
只提交清单与原创分析，不复制上游源码、字体或固件。研究检出保存在忽略目录或 /workspace/work/official。
本项目 fetch_official.py 只克隆已锁定提交，不自动安装/初始化子模块或执行第三方代码。
许可证获确认后，再评估 Git submodule、补丁集与发布义务，不移动黄金候选。
