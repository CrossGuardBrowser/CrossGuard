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

### 2.4 Integration with Existing Code

**In `launcher.py`, add new routes:**

```python
elif p == "/api/platforms":
    self._api_get_platforms()
elif p.startswith("/api/profile/") and p.endswith("/platform"):
    self._api_set_platform(self._id_from_path())
elif p.startswith("/api/profile/") and p.endswith("/ua"):
    self._api_set_ua(self._id_from_path())
```

**New handler methods in `Handler` class:**

```python
def _api_get_platforms(self):
    self._json({
        "platforms": [...],  # platform list
        "default": "windows",
        "uaByPlatform": PLATFORM_UA_PRESETS
    })

def _api_set_platform(self, pid):
    length = int(self.headers.get("Content-Length", 0))
    body = json.loads(self.rfile.read(length).decode("utf-8"))
    platform_id = body.get("platform", "windows")

    if platform_id not in PLATFORM_UA_PRESETS:
        self._json({"error": "invalid platform"}, 400)
        return

    data = load_profiles()
    profile = find_profile(data, pid)
    if not profile:
        self._json({"error": "profile not found"}, 404)
        return

    apply_platform_to_config(profile["config"], platform_id)
    save_profiles(data)

    self._json({
        "ok": True,
        "ua": PLATFORM_UA_PRESETS[platform_id],
        "platformValue": PLATFORM_VALUE_MAP[platform_id],
        "source": "platform"
    })

def _api_set_ua(self, pid):
    length = int(self.headers.get("Content-Length", 0))
    body = json.loads(self.rfile.read(length).decode("utf-8"))
    ua = body.get("ua", "")
    source = body.get("source", "custom")

    data = load_profiles()
    profile = find_profile(data, pid)
    if not profile:
        self._json({"error": "profile not found"}, 404)
        return

    profile["config"]["userAgent"] = {
        "type": 2,
        "ua": ua,
        "source": source
    }
    save_profiles(data)
    self._json({"ok": True})
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

The new platform selector sits above the existing UserAgent module in the fingerprint section. It replaces/manages the UA field when active.

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

**DOM structure in `renderModules()`:**

```javascript
// Add new module after userAgent in MODULES array
const PLATFORM_MODULE = {
  key: '_platformSelector',
  label: '平台选择',
  modes: [{v: 0, l: '默认'}, {v: 1, l: '选择平台'}, {v: 2, l: '自定义UA'}],
  hint: '选择平台后 UserAgent 和 Platform 将自动更新为该平台的预设值'
};
```

### 3.3 State Management

**Global state variables (existing):**
```javascript
let profiles = [];
let currentProfile = null;
let config = {};           // alias to currentProfile.config
let proxyCheckResult = null;
```

**New state:**
```javascript
let platformPresets = {};  // { windows: "...ua...", mac: "...ua..." }
let currentPlatform = 'windows';  // active platform id
let uaSource = 'platform';  // 'platform' | 'custom'
```

**Platform data fetched on editor open:**
```javascript
async function editProfile(id) {
  // ... load profile ...
  await fetchPlatformPresets();  // new
  showEditorView();
}

async function fetchPlatformPresets() {
  try {
    const r = await fetch('/api/platforms');
    const data = await r.json();
    platformPresets = data.uaByPlatform;
  } catch(e) {
    platformPresets = {};
  }
}
```

### 3.4 Platform Selector Render Logic

In `createModuleRow()` for the platform selector module:

```javascript
function createModuleRow(mod) {
  if (mod.key === '_platformSelector') {
    return createPlatformSelectorRow(mod);
  }
  // ... existing logic ...
}

function createPlatformSelectorRow(mod) {
  // Determine current mode from config state
  const currentMode = getModuleType(mod);  // 0=default, 1=platform, 2=custom

  // If userAgent.source === 'custom', force mode 2
  const effectiveMode = config.userAgent?.source === 'custom' ? 2 : currentMode;

  let html = `<div class="module-row" id="mod-_platformSelector">`;

  // Header with mode buttons
  html += `<div class="module-header">
    <span class="module-label">${mod.label}</span>
    <div class="module-modes">
      <button class="mode-btn${effectiveMode===0?' active':''}" onclick="setPlatformMode(0)">默认</button>
      <button class="mode-btn${effectiveMode===1?' active':''}" onclick="setPlatformMode(1)">选择平台</button>
      <button class="mode-btn${effectiveMode===2?' active':''}" onclick="setPlatformMode(2)">自定义</button>
    </div>
  </div>`;

  // Hint
  html += `<div class="module-config show" style="border-top:none;margin-top:6px;padding-top:0">
    <div style="font-size:12px;color:#909399;padding:0 0 0 130px">💡 ${mod.hint}</div>
  </div>`;

  // Mode-specific config area
  html += `<div class="module-config show">`;

  if (effectiveMode === 1) {
    // Platform dropdown
    html += `<div class="config-row">
      <span class="config-label">平台</span>
      <select class="config-input wide" id="platform-select" onchange="onPlatformChange(this.value)">`;
    const platforms = ['windows','mac','linux','android','ios'];
    const labels = {windows:'Windows 🪟', mac:'Mac 🍎', linux:'Linux 🐧', android:'Android 🤖', ios:'iOS 📱'};
    for (const p of platforms) {
      html += `<option value="${p}"${p===currentPlatform?' selected':''}>${labels[p]}</option>`;
    }
    html += `</select></div>`;

    // UA preview (read-only)
    html += `<div class="config-row">
      <span class="config-label">UA 预览</span>
      <input type="text" class="config-input wide" id="ua-preview"
        value="${platformPresets[currentPlatform] || ''}"
        readonly style="background:#fafafa;color:#909399">
    </div>`;

    // Sync button
    html += `<div class="config-row">
      <span class="config-label"></span>
      <button class="btn-sm" onclick="syncPlatformToUA()">应用平台设置</button>
    </div>`;
  } else if (effectiveMode === 2) {
    // Custom UA textarea
    html += `<div class="config-row">
      <span class="config-label">UA 字符串</span>
      <textarea class="config-textarea" id="custom-ua-input"
        placeholder="Mozilla/5.0 ..."
        oninput="onCustomUAInput(this.value)">${config.userAgent?.ua || ''}</textarea>
    </div>`;
    html += `<div style="font-size:12px;color:#E6A23C;padding:4px 0 0 90px">
      ⚠️ 自定义 UA 已脱离平台关联
    </div>`;
  }

  html += `</div></div>`;
  return html;
}
```

### 3.5 Event Handlers

```javascript
function setPlatformMode(mode) {
  if (mode === 0) {
    // Default: clear platform link, keep UA as-is with source=custom
    config.userAgent = config.userAgent || { type: 2, ua: '' };
    config.userAgent.source = 'custom';
    currentPlatform = 'windows';
  } else if (mode === 1) {
    // Platform mode: apply current platform to UA
    applyPlatformToConfig(currentPlatform);
  } else if (mode === 2) {
    // Custom mode: mark as custom, stop syncing
    if (config.userAgent) config.userAgent.source = 'custom';
  }
  const row = document.getElementById('mod-_platformSelector');
  row.replaceWith(createPlatformSelectorRow({ key: '_platformSelector', label: '平台选择', modes: [...], hint: '...' }));
  renderSummary();
}

function onPlatformChange(platformId) {
  currentPlatform = platformId;
  // Update preview
  const preview = document.getElementById('ua-preview');
  if (preview && platformPresets[platformId]) {
    preview.value = platformPresets[platformId];
  }
}

function syncPlatformToConfig() {
  // Apply selected platform to config
  const ua = platformPresets[currentPlatform];
  if (!ua) return;

  config.userAgent = {
    type: 2,
    ua: ua,
    source: 'platform'
  };
  config.platform = {
    type: 2,
    value: PLATFORM_VALUE_MAP[currentPlatform]
  };

  // Re-render userAgent and platform rows
  const uaRow = document.getElementById('mod-userAgent');
  if (uaRow) uaRow.replaceWith(createModuleRow(MODULES.find(m => m.key === 'userAgent')));
  const platRow = document.getElementById('mod-platform');
  if (platRow) platRow.replaceWith(createModuleRow(MODULES.find(m => m.key === 'platform')));

  renderSummary();
  toast('已应用平台设置');
}

function onCustomUAInput(ua) {
  if (config.userAgent) {
    config.userAgent.ua = ua;
    config.userAgent.source = 'custom';
  } else {
    config.userAgent = { type: 2, ua: ua, source: 'custom' };
  }
  renderSummary();
}
```

### 3.6 Platform → Chrome value mapping (JS)

```javascript
const PLATFORM_VALUE_MAP = {
  windows: 'Win64',
  mac: 'MacIntel',
  linux: 'Linux x86_64',
  android: 'Linux armv7l',
  ios: 'iPhone'
};
```

### 3.7 Summary Panel Updates

In `renderSummary()`, add platform display:

```javascript
// Add after proxy items
const platformDisplay = config.userAgent?.source === 'platform'
  ? `${currentPlatform.toUpperCase()} 平台 UA`
  : (config.platform?.value || '默认');
html += `<div class="summary-item">
  <span class="s-key">平台:</span>
  <span class="s-val">${platformDisplay}</span>
</div>`;
```

### 3.8 Existing UserAgent and Platform Module Behavior

**When platform selector mode = 1 (platform mode):**
- `mod-userAgent` row: rendered as read-only textarea showing preset UA
- `mod-platform` row: hidden or shown as read-only with preset value

**When platform selector mode = 2 (custom mode):**
- `mod-userAgent` row: normal editable textarea, user can type custom UA
- `mod-platform` row: normal editable select

**When platform selector mode = 0 (default):**
- Both modules behave as before (type=0 or type=2 with manual entry)

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
    },

    "proxy": { ... },
    "timezone": { ... },
    ...
  }
}
```

### 4.2 Source Field Semantics

| `userAgent.source` | Meaning | Behavior |
|---|---|---|
| `"platform"` | UA driven by platform selector | Platform change updates UA automatically |
| `"custom"` | User manually entered UA | Platform selector shows "disconnected" state |
| absent/`undefined` | Legacy (pre-feature) config | Treated as `custom` for backward compat |

### 4.3 Migration Strategy

When loading a profile without `userAgent.source`:
```javascript
function normalizeConfig(config) {
  if (!config.userAgent) return config;
  if (!config.userAgent.source) {
    config.userAgent.source = 'custom';  // treat existing UAs as custom
  }
  return config;
}
```

On `editProfile()`:
```javascript
currentProfile.config = normalizeConfig(currentProfile.config);
config = currentProfile.config;
```

---

## 5. Security Considerations

### 5.1 Input Validation (Backend)

```python
# In _api_set_platform
platform_id = body.get("platform", "windows")
if platform_id not in PLATFORM_UA_PRESETS:
    self._json({"error": "invalid platform"}, 400)
    return

# In _api_set_ua
ua = body.get("ua", "")
if len(ua) > 500 or len(ua) < 10:
    self._json({"error": "ua string too short or too long"}, 400)
    return
```

### 5.2 XSS Prevention (Frontend)

All UA strings displayed in the UI are escaped via the existing `esc()` function:
```javascript
function esc(s) { const d=document.createElement('div'); d.textContent=s||''; return d.innerHTML; }
```

The `ua-preview` input is `readonly` in platform mode, preventing user injection.

### 5.3 CSRF

The launcher HTTP service runs on `127.0.0.1` only, not exposed externally. CSRF is not a significant concern for localhost-only API.

---

## 6. Error Handling

### 6.1 Backend Error → API Response → Frontend

| Error | HTTP Code | Response | Frontend Handling |
|---|---|---|---|
| Profile not found | 404 | `{"error": "not found"}` | `toast('加载失败', true)` |
| Invalid platform | 400 | `{"error": "invalid platform"}` | `toast('无效平台', true)` |
| UA too long | 400 | `{"error": "ua string too long"}` | `toast('UA 超出长度限制', true)` |
| Server error | 500 | `{"error": "..."}` | `toast('服务器错误', true)` |

### 6.2 Network Errors

```javascript
async function fetchPlatformPresets() {
  try {
    const r = await fetch('/api/platforms');
    if (!r.ok) throw new Error('failed');
    platformPresets = await r.json();
  } catch(e) {
    platformPresets = {};
    toast('加载平台预设失败', true);
  }
}
```

---

## 7. Risk Assessment

### 7.1 Technical Risks

| Risk | Likelihood | Impact | Mitigation |
|---|---|---|---|
| Chrome version drift (UA string becomes outdated) | Medium | Low | UA presets use Chrome 114 as baseline; add version comment; user can override |
| Platform selector state drifts from actual UA after manual edit | Medium | Medium | `source` field always reflects truth; UI shows "disconnected" badge when `source=custom` |
| Large UA strings bloat profile JSON | Low | Low | 500 char limit enforced on backend; typical UA ~150 chars |
| Breaking existing profiles without `source` field | Low | Low | Normalize on load; default to `custom` |

### 7.2 Migration Risks

| Risk | Mitigation |
|---|---|
| Existing profiles with type=2 UA configs need `source` field | Auto-normalize on `editProfile()` |
| Users who manually edited UA expect platform selector to not override | Default mode is `custom` (no auto-sync); user must explicitly opt-in to platform mode |

---

## 8. Implementation Checklist

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

### Testing
- [ ] Test platform switching updates UA in preview
- [ ] Test "应用平台设置" syncs UA and platform.value to config
- [ ] Test switching to custom mode shows disconnect warning
- [ ] Test summary panel shows correct platform
- [ ] Test profile save/load roundtrip preserves platform and source
- [ ] Test existing profiles (no source field) load correctly
