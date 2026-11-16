# CrossGuard 官方站点

CrossGuard 开源指纹浏览器产品官网（纯静态，零构建）。

## 目录结构
```
CrossGuardWebsite/
├── index.html          # 单页站点
├── css/style.css       # 深色科技风样式
├── js/main.js          # 滚动进度/导航/显现动画/计数/FAQ
├── assets/favicon.svg  # 站点图标
└── README.md
```

## 本地预览
无需构建，任选其一：
```bash
# 方式 1：Python
python -m http.server 8000

# 方式 2：直接双击 index.html 用浏览器打开
```
然后访问 `http://localhost:8000`。

## 部署
纯静态文件，可托管在任意静态服务：
- **GitHub Pages**：把本目录推到仓库，开启 Pages 即可。
- **Vercel / Netlify / Cloudflare Pages**：直接拖入或连接仓库。
- **Nginx**：把目录作为 root。

## 内容来源
- 项目信息：CrossGuard v2.4.7（Chromium 138，多环境隔离，UA / Client Hints / 语言 / Canvas / WebGL 噪音按同一次启动对齐，36 个 MCP 工具，RPA，云端同步）。
- 版本与下载：[github.com/sunny5156/CrossGuard/releases](https://github.com/sunny5156/CrossGuard/releases)

## 自定义
- 颜色主题：改 `css/style.css` 顶部 `:root` 变量（`--cyan/--blue/--violet`）。
- 版本信息：改 `index.html` 的 `#releases` 区块与 hero badge。
- 下载链接：全局指向 `…/releases/latest`，替换为自有分发地址即可。
