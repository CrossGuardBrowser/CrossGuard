<p align="center">
  <img src="ui/crossguard.png" alt="CrossGuard Logo" width="120">
</p>

<h1 align="center">CrossGuard</h1>

<p align="center">
  基于 Chromium 138 的开源指纹浏览器，支持多环境隔离与全参数指纹配置
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Chromium-138-blue" alt="Chromium Version">
  <img src="https://img.shields.io/badge/Version-2.2.39-brightgreen" alt="Version">
  <img src="https://img.shields.io/badge/Platform-Windows-green" alt="Platform">
  <img src="https://img.shields.io/badge/License-GPL-orange" alt="License">
  <img src="https://img.shields.io/badge/MCP-AI%20Automation-purple" alt="MCP">
</p>

---

## 功能特性

- **多环境隔离** — 每个环境独立的 Cookie、历史记录、LocalStorage，互不干扰
- **全参数指纹配置** — 覆盖 25+ 浏览器指纹维度，支持自定义/噪音/跟随IP 多种模式
- **🤖 MCP AI 自动化** — 内置 MCP 服务，让 AI（Claude / 任意 MCP 客户端）远程「选环境 → 启动 → 控制浏览器」，13 个工具实现导航/执行脚本/截图/Cookie/标签页全流程自动化
- **代理支持** — HTTP / HTTPS / SOCKS4 / SOCKS5，支持用户名密码认证
- **IP 智能联动** — 代理启用时自动根据出口 IP 设置时区、语言、地理位置、语音
- **WebRTC 防护** — 检测到代理时自动保护真实 IP 不泄露
- **DNS 防泄露** — 内置 DNS 泄露防护，杜绝代理下的 DNS 请求外泄
- **设备指纹伪装** — 主机名 / MAC 地址 / SSL-TLS 握手 / 硬件加速全链路伪装
- **UACH 自洽** — 完整 User-Agent Client Hints，平台版本与 UA 始终一致，绕过现代指纹检测
- **可视化管理** — Web UI 管理所有环境，一键创建/编辑/启动
- **指纹真实度高** — BrowserScan 检测接近 100% 真实度（Canvas/WebGL 无篡改痕迹）
- **重型网站稳定** — 深度适配飞书等重型 SPA，长时间运行不崩溃

## 截图

### 环境管理

<img src="assets/img-001.png" alt="环境管理" width="800">

### 指纹配置

<img src="assets/img-002.png" alt="指纹配置" width="800">

### 环境首页

<img src="assets/img-003.png" alt="环境首页" width="800">

### 指纹检测 (BrowserScan 97%)

<img src="assets/img-004.png" alt="指纹检测" width="800">

## 支持的指纹参数

| 分类 | 参数 | 模式 |
|------|------|------|
| 基础信息 | UserAgent | 默认 / 自定义 |
| 基础信息 | UACH / Client Hints | 跟随 UA / 自定义 |
| 基础信息 | Platform | 默认 / 自定义 |
| 基础信息 | 语言 | 默认 / 本机 / 基于IP |
| 基础信息 | 时区 | 默认 / 自定义 / 跟随IP |
| 基础信息 | Do Not Track | 默认 / 本机 / 自定义 |
| 网络 | 代理 | 直连 / HTTP / HTTPS / SOCKS4 / SOCKS5 |
| 网络 | WebRTC | 关闭 / 替换IP / 禁用UDP / 仅公共接口 |
| 网络 | 地理位置 | 默认 / 本机 / 基于IP |
| 网络 | DNS 泄露防护 | 开 / 关 |
| 网络 | SSL / TLS 指纹 | 默认 / 自定义 |
| 硬件 | 分辨率 | 默认 / 本机 |
| 硬件 | CPU / 内存 | 默认 / 本机 |
| 硬件 | 媒体设备 | 默认 / 本机 |
| 硬件 | 设备名 / 主机名 | 默认 / 自定义 |
| 硬件 | MAC 地址 | 默认 / 噪音 / 自定义 |
| 硬件 | 硬件加速 | 开 / 关 |
| 渲染 | Canvas | 默认 / 本机 / 噪音 |
| 渲染 | WebGL | 默认 / 本机 / 噪音 |
| 渲染 | GupGL | 默认 / 本机 / 噪音 |
| 渲染 | ClientRects | 默认 / 本机 / 噪音 |
| 音频 | AudioContext | 默认 / 本机 / 噪音 |
| 字体 | 字体列表 | 默认 / 本机 / 噪音 |
| 语音 | SpeechVoices | 默认 / 本机 / 噪音 / 基于IP |
| 安全 | 端口保护 | 默认 / 本机 |

## 🤖 MCP AI 自动化

CrossGuard 内置 MCP（Model Context Protocol）服务，让 AI 客户端（Claude / Cursor / 任意 MCP Host）**远程控制各指纹环境浏览器**，实现端到端自动化。

**13 个工具**：环境管理（`list_environments` / `launch_environment` / `close_environment` / `list_running`）+ 页面自动化（`navigate` / `evaluate_script` / `screenshot` / `get_cookies` / `set_cookies` / `list_tabs` / `new_tab` / `close_tab`）。

在 AI 客户端配置 `.mcp.json`：

```json
{
  "mcpServers": {
    "crossguard": {
      "url": "http://<机器IP>:18901/mcp",
      "headers": { "Authorization": "Bearer <Token>" }
    }
  }
}
```

Token 在管理界面「MCP 服务」卡片一键生成/重置。典型流程：`list_environments → launch_environment(id) → navigate → evaluate_script → screenshot → close_environment`。

## 快速开始

### 环境要求

- Windows 10/11 (64-bit)
- Python >= 3.10
- `pip install pycryptodome`

### 运行

```bash
cd CrossGuardLauncher/

# 启动管理界面 + Chrome
python launcher.py

# 仅启动 HTTP 服务
python launcher.py --no-launch

# 指定 chrome 路径和端口
python launcher.py --chrome "path/to/chrome.exe" --port 18900
```

启动后访问 http://127.0.0.1:18900 打开管理界面。

### 打包

```bash
cd CrossGuardLauncher/
pip install pyinstaller

# 打包 exe + 免安装包
python build.py

# 生成安装程序 (需要 Inno Setup 6)
ISCC.exe setup.iss
```

## 项目结构

```
CrossGuard/                 # Chromium 138 补丁文件 (覆盖到 chromium/src/)
├── chrome/                 #   浏览器进程补丁
│   └── app/                #     启动入口、AES 加解密、指纹 JSON 解析
├── third_party/blink/      #   Blink 渲染引擎补丁 (指纹注入核心)
├── content/                #   字体代理、Mojo IPC
├── components/             #   UA、语言、权限补丁
├── base/                   #   指纹全局单例
├── cc/                     #   命令行 switch 定义
├── net/                    #   主机名、MAC 地址欺骗
├── ui/                     #   分辨率欺骗、Logo
└── assets/                 #   截图资源

CrossGuardLauncher/         # Python 启动器 (独立目录)
├── launcher.py             #   HTTP 服务 + Chrome 环境管理
├── config.html             #   Web 管理界面
├── build.py                #   打包脚本
└── docs/                   #   开发文档
```

详细文档见 [`CrossGuardLauncher/docs/`](../CrossGuardLauncher/docs/):
- [编译打包指南](../CrossGuardLauncher/docs/BUILD.md)
- [架构说明](../CrossGuardLauncher/docs/ARCHITECTURE.md)
- [配置路径说明](../CrossGuardLauncher/docs/CONFIG.md)
- [二次开发指南](../CrossGuardLauncher/docs/DEVELOPMENT.md)

## 联系我们

<img src="assets/img-005.png" alt="指纹检测" width="800">

> 请注意保护您的隐私和安全，在加入公开讨论群组时谨慎提供个人信息。

---

**License**: GPL
