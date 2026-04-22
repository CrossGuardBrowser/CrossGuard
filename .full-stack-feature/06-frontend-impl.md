# Frontend Implementation: 平台选择,对应的UA也要跟随变化

## Files Modified

### `launcher/config.html`

## Changes

### 1. New Constants Added

```javascript
// Platform selector constants
const PLATFORM_VALUE_MAP = {
  windows: 'Win64',
  mac: 'MacIntel',
  linux: 'Linux x86_64',
  android: 'Linux armv7l',
  ios: 'iPhone',
};
```

### 2. New Global State Variables

```javascript
let platformPresets = {};     // { windows: "...ua...", mac: "...ua..." }
let currentPlatform = 'windows';  // active platform id
let uaSource = 'platform';    // 'platform' | 'custom'
```

### 3. New Module Definition

Added `_platformSelector` as first module in MODULES array:
```javascript
{key:'_platformSelector',label:'平台选择',hint:'选择平台后 UserAgent 和 Platform 将自动更新为该平台的预设值'}
```

### 4. New Functions

| Function | Description |
|----------|-------------|
| `normalizeConfig(cfg)` | Backward-compatible config migration, sets currentPlatform/uaSource from config |
| `detectPlatformFromUA(ua)` | Detects platform ID from UA string |
| `fetchPlatformPresets()` | Fetches platform list and UA presets from `/api/platforms` |
| `createPlatformSelectorRow(mod)` | Renders the platform selector UI |
| `onPlatformChange(platformId)` | Handles platform dropdown change |
| `syncPlatformToConfig()` | Applies selected platform to config and re-renders |
| `onCustomUAInput(ua)` | Handles custom UA input |
| `onCustomModeSelected()` | Switches to custom UA mode |

### 5. Modified Functions

| Function | Change |
|----------|--------|
| `editProfile(id)` | Added `normalizeConfig()` and `await fetchPlatformPresets()` calls |
| `createModuleRow(mod)` | Added special handling for `_platformSelector` module |
| `renderSummary()` | Added platform info display when `userAgent.source === 'platform'` |

### 6. Platform Selector UI

**Mode: 选择平台 (Platform Mode)**
- Platform dropdown (Windows 🪟, Mac 🍎, Linux 🐧, Android 🤖, iOS 📱)
- Read-only UA preview input
- "应用平台设置" button to apply

**Mode: 自定义 (Custom Mode)**
- Editable UA textarea
- Warning badge: "⚠️ 自定义 UA 已脱离平台关联"

### 7. Integration Points

- Platform selector module inserted before UserAgent in fingerprint section
- When platform mode active, UserAgent and Platform modules show preset values
- Summary panel shows platform emoji + "平台 UA" when in platform mode

### 8. Backward Compatibility

- Existing profiles without `platformSelector` get default values
- Existing profiles without `userAgent.source` treated as custom
- All existing functionality preserved
