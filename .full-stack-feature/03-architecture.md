# Platform Selector Architecture Design

## 1. Overview

### 1.1 Purpose
Add a platform selector to the CrossGuard fingerprint browser that allows users to switch between Windows/Mac/Linux/Android/iOS platforms with automatic UA string matching.

### 1.2 Existing Architecture
- **Backend**: Python HTTP server (`launcher.py`) with `http.server.HTTPServer`
- **Frontend**: Pure HTML/CSS/JS (`config.html`), no framework
- **Storage**: `profiles.json` (JSON flat file)
- **Config flow**: Python config → AES encrypted JSON → Chrome C++ → Mojo IPC → Blink renderer
- **Existing platform field**: `config.platform.value` stores raw platform strings (`Win32`, `MacIntel`, etc.)

### 1.3 Design Principles
- Non-breaking: Existing config schema extended, not replaced
- Backward compatible: `type=0` (default) still works without platform selector
- Auto-sync: Platform change triggers UA update by default
- Source tracking: `userAgent.source` field tracks whether UA came from platform preset, custom input, or global default

---

## 2. Backend Architecture

### 2.1 Platform UA Presets

Static data embedded in `launcher.py`:

```python
# Platform → UA mapping (Chrome 114 on each platform)
PLATFORM_UA_PRESETS = {
    "windows": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.0.0 Safari/537.36",
    "mac":     "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.0.0 Safari/537.36",
    "linux":   "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.0.0 Safari/537.36",
    "android": "Mozilla/5.0 (Linux; Android 13; Pixel 7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.0.0 Mobile Safari/537.36",
    "ios":     "Mozilla/5.0 (iPhone; CPU iPhone OS 16_6 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/16.6 Mobile/15E148 Safari/604.1",
}

# Platform → platform.value mapping (what Chrome reports)
PLATFORM_VALUE_MAP = {
    "windows": "Win64",
    "mac":     "MacIntel",
    "linux":   "Linux x86_64",
    "android": "Linux armv7l",
    "ios":     "iPhone",
}
```

### 2.2 New API Endpoints

#### GET /api/platforms
Returns available platforms and current default.

**Response 200:**
```json
{
  "platforms": [
    { "id": "windows", "label": "Windows", "icon": "🪟" },
    { "id": "mac",     "label": "Mac",     "icon": "🍎" },
    { "id": "linux",   "label": "Linux",   "icon": "🐧" },
    { "id": "android", "label": "Android", "icon": "🤖" },
    { "id": "ios",     "label": "iOS",     "icon": "📱" }
  ],
  "default": "windows",
  "uaByPlatform": {
    "windows": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 ...",
    "mac": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 ..."
  }
}
```

#### PUT /api/profile/{id}/platform
Set profile platform. Returns the auto-computed UA and platform value.

**Request body:**
```json
{ "platform": "mac" }
```

**Response 200:**
```json
{
  "ok": true,
  "ua": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 ...",
  "platformValue": "MacIntel",
  "source": "platform"
}
```

**Response 404:** `{ "error": "profile not found" }`
**Response 400:** `{ "error": "invalid platform" }`

#### PUT /api/profile/{id}/ua
Set UA manually (breaks link with platform selector).

**Request body:**
```json
{ "ua": "Mozilla/5.0 ...", "source": "custom" }
```

**Response 200:**
```json
{ "ok": true }
```

### 2.3 Service Layer

#### Module: `platform_service.py` (new file)

```python
# platform_service.py

PLATFORM_UA_PRESETS = { ... }  # as defined above
PLATFORM_VALUE_MAP = { ... }    # as defined above

def get_platform_presets():
    """Return list of available platforms with metadata."""
    return [...]

def resolve_ua_for_platform(platform_id: str) -> str:
    """Return the preset UA string for a platform."""
    return PLATFORM_UA_PRESETS.get(platform_id, PLATFORM_UA_PRESETS["windows"])

def resolve_platform_value(platform_id: str) -> str:
    """Return the Chrome platform value string for a platform."""
    return PLATFORM_VALUE_MAP.get(platform_id, "Win64")

def apply_platform_to_config(config: dict, platform_id: str):
    """
    Apply platform selection to a profile config dict.
    Sets userAgent.ua, userAgent.source, platform.value.
    """
    ua = resolve_ua_for_platform(platform_id)
    plat_val = resolve_platform_value(platform_id)

    config["userAgent"] = {
        "type": 2,
        "ua": ua,
        "source": "platform"
    }
    config["platform"] = {
        "type": 2,      # type 2 = custom (not default)
        "value": plat_val
    }
    return config

def apply_custom_ua_to_config(config: dict, ua: str):
    """
    Apply a custom UA, breaking the platform link.
    Sets userAgent.ua and source='custom', preserves platform.value.
    """
    config["userAgent"] = {
        "type": 2,
        "ua": ua,
        "source": "custom"
    }
    return config
```

---

## 3. Frontend Architecture

### 3.1 Component Hierarchy

```
config.html (single page)
├── Sidebar
│   ├── Logo + Title
│   ├── Nav: 环境管理 / 云端同步
│   └── Cloud status / user info
├── Profile List View (#view-list)
│   ├── Header: 环境管理 + 新建按钮
│   └── Table: ID, 名称, 代理, IP, 时间, 操作
├── Profile Editor View (#view-editor)
│   ├── Header: 返回 + 导入/导出
│   ├── Name Row
│   ├── Section: 网络代理 (ProxyModule)
│   ├── Section: Cookie (CookieModule)
│   ├── Section: 指纹配置
│   │   ├── PlatformSelectorModule  [NEW]
│   │   ├── UserAgentModule
│   │   ├── PlatformModule (existing, hidden when platform selector active)
│   │   └── ... other modules
│   └── Footer: 保存 / 保存并启动 / 重置
└── Summary Panel (#summary-panel)
    ├── Header: 概要 + 生成指纹
    └── Items list
```

### 3.2 PlatformSelectorModule Component

**UI Structure:**
```
┌─────────────────────────────────────────────────────────────┐
│  平台选择                                                     │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ [🪟 Windows ▼]  ○自动跟随平台   ○自定义UA              │  │
│  └──────────────────────────────────────────────────────┘  │
│  💡 选择平台后 UserAgent 和 Platform 将自动更新              │
└─────────────────────────────────────────────────────────────┘
```

**States:**
1. **Auto mode (default)**: Platform dropdown active, UA field shown but read-only, driven by platform selection
2. **Custom mode**: Platform dropdown disabled, UA field editable, "脱离平台" badge shown

---

## 4. Data Model

### 4.1 Extended Profile Config Schema

```json
{
  "id": 1,
  "name": "环境 1",
  "createdAt": "2026-03-12 19:28",
  "config": {
    "init": 2,

    "userAgent": {
      "type": 2,
      "ua": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) ...",
      "source": "platform"
    },

    "platform": {
      "type": 2,
      "value": "MacIntel"
    }
  }
}
```

### 4.2 Source Field Semantics

| `userAgent.source` | Meaning | Behavior |
|---|---|---|
| `"platform"` | UA driven by platform selector | Platform change updates UA automatically |
| `"custom"` | User manually entered UA | Platform selector shows "disconnected" state |
| absent/`undefined` | Legacy (pre-feature) config | Treated as `custom` for backward compat |

---

## 5. Security Considerations

- **Input Validation**: Backend validates platform ID against whitelist and UA string length (10-500 chars)
- **XSS Prevention**: All UA strings escaped via existing `esc()` function
- **CSRF**: Not a concern - localhost-only API on 127.0.0.1

---

## 6. Error Handling

| Error | HTTP Code | Response | Frontend Handling |
|---|---|---|---|
| Profile not found | 404 | `{"error": "not found"}` | `toast('加载失败', true)` |
| Invalid platform | 400 | `{"error": "invalid platform"}` | `toast('无效平台', true)` |
| UA too long | 400 | `{"error": "ua string too long"}` | `toast('UA 超出长度限制', true)` |
| Server error | 500 | `{"error": "..."}` | `toast('服务器错误', true)` |

---

## 7. Implementation Checklist

### Backend
- [ ] Add `PLATFORM_UA_PRESETS` and `PLATFORM_VALUE_MAP` to `launcher.py`
- [ ] Add `platform_service.py` with helper functions
- [ ] Add `GET /api/platforms` route and handler
- [ ] Add `PUT /api/profile/{id}/platform` route and handler
- [ ] Add `PUT /api/profile/{id}/ua` route and handler
- [ ] Add input validation for platform ID and UA string length

### Frontend
- [ ] Add `PLATFORM_MODULE` definition to `MODULES` array
- [ ] Add `PLATFORM_VALUE_MAP` constant to JS
- [ ] Add `platformPresets`, `currentPlatform`, `uaSource` to global state
- [ ] Add `fetchPlatformPresets()` call in `editProfile()`
- [ ] Add `createPlatformSelectorRow()` function
- [ ] Add `setPlatformMode()`, `onPlatformChange()`, `syncPlatformToConfig()`, `onCustomUAInput()` handlers
- [ ] Update `renderSummary()` to show platform info
- [ ] Update `normalizeConfig()` for backward compat
- [ ] Add CSS for platform selector component
