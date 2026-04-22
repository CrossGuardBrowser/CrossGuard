# Requirements: 平台选择,对应的UA也要跟随变化

## Problem Statement

用户在使用 CrossGuard 指纹浏览器时，需要在多平台（Windows/Mac/Linux/Android/iOS）指纹配置间切换。当前手动设置UA容易出错，通过平台选择器可实现一键切换，UA自动匹配对应平台的预设值。

## Acceptance Criteria

- [x] 平台选择器UI - Web界面上有平台选择下拉菜单
- [x] UA自动切换 - 选择平台后UA字符串自动更新
- [x] 预设UA列表 - 支持Windows/Mac/Linux/Android/iOS等平台
- [x] 配置持久化 - 平台选择和UA配置保存在配置文件中

## Scope

### In Scope

- 在现有 launcher Web UI 上扩展平台选择功能
- 实现预设UA字符串与平台关联
- 平台切换时自动更新UA
- 将平台选择和UA配置存储到现有指纹配置文件

### Out of Scope

- 云端UA同步
- 指纹模板管理
- 用户自定义UA输入
- 浏览器内核切换

## Technical Constraints

- 继续使用现有 Python HTTP 服务架构
- 兼容现有的指纹注入机制
- 在现有 launcher Web UI 上扩展

## Technology Stack

- **Frontend**: HTML/CSS/JS (现有 Web UI 扩展)
- **Backend**: Python (launcher HTTP 服务)
- **Storage**: JSON 配置文件 (现有 fingerprint 配置系统)
- **Integration**: 通过现有指纹配置机制挂载UA

## Dependencies

- 依赖现有的指纹配置系统存储机制
- 通过 CDP 协议设置 UA (如有需要)

## Configuration

- Stack: auto-detect (Python + Web UI)
- API Style: REST (内部 HTTP API)
- Complexity: medium

## Platform UA Mapping

| Platform   | Example UA (Chrome on Windows)                              |
|------------|------------------------------------------------------------|
| Windows    | Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 |
| Mac        | Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 |
| Linux      | Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36       |
| Android    | Mozilla/5.0 (Linux; Android 13; Pixel 7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.0.0 Mobile |
| iOS        | Mozilla/5.0 (iPhone; CPU iPhone OS 16_6 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/16.6 Mobile/15E148 Safari/604.1 |
