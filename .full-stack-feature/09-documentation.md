# Documentation: 平台选择,对应的UA也要跟随变化

## Feature Overview

Allows users to select a platform (Windows/Mac/Linux/Android/iOS) and automatically update the User-Agent string to match the selected platform.

## User Flow

1. Open profile editor in CrossGuard Web UI
2. Find "平台选择" (Platform Selector) module at top of fingerprint section
3. Choose mode:
   - **选择平台 (Select Platform)**: Choose from dropdown, click "应用平台设置" to apply
   - **自定义 (Custom)**: Manually enter custom UA string
4. Platform and UA are saved with profile

## API Documentation

### GET /api/platforms

Returns available platforms with metadata and UA presets.

**Response:**
```json
{
  "platforms": [
    {"id": "windows", "label": "Windows", "icon": "🪟", "order": 1},
    {"id": "mac", "label": "Mac", "icon": "🍎", "order": 2},
    {"id": "linux", "label": "Linux", "icon": "🐧", "order": 3},
    {"id": "android", "label": "Android", "icon": "🤖", "order": 4},
    {"id": "ios", "label": "iOS", "icon": "📱", "order": 5}
  ],
  "default": "windows",
  "uaByPlatform": {
    "windows": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36...",
    ...
  }
}
```

### PUT /api/profile/{id}/platform

Sets profile platform and auto-updates UA.

**Request:**
```json
{"platform": "mac"}
```

**Response:**
```json
{
  "ok": true,
  "ua": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36...",
  "platformValue": "MacIntel",
  "source": "platform"
}
```

### PUT /api/profile/{id}/ua

Sets custom UA for a profile.

**Request:**
```json
{"ua": "Mozilla/5.0 ...", "source": "custom"}
```

## Platform UA Mapping

| Platform | UA Preview | Platform Value |
|----------|-----------|----------------|
| Windows 🪟 | Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36... | Win64 |
| Mac 🍎 | Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36... | MacIntel |
| Linux 🐧 | Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36... | Linux x86_64 |
| Android 🤖 | Mozilla/5.0 (Linux; Android 13; Pixel 7) AppleWebKit/537.36... | Linux armv7l |
| iOS 📱 | Mozilla/5.0 (iPhone; CPU iPhone OS 16_6 like Mac OS X)... | iPhone |

## Configuration Schema

```json
{
  "userAgent": {
    "type": 2,
    "ua": "...",
    "source": "platform"
  },
  "platform": {
    "type": 2,
    "value": "MacIntel"
  },
  "platformSelector": {
    "type": "enum",
    "value": "mac",
    "autoUpdate": true
  }
}
```

## Architecture Decision Record

### Decision: Static UA Presets Instead of Dynamic Lookup

**Context:** We needed to map platform selections to valid Chrome UA strings.

**Decision:** Use static embedded presets in Python code rather than external API or user-configurable templates.

**Rationale:**
- Simplicity: No external dependencies
- Consistency: All installations share same presets
- Performance: O(1) dictionary lookup
- Maintenance: Chrome version updates require code change anyway

### Decision: Source Field Tracking

**Context:** We needed to track whether UA came from platform preset or manual entry.

**Decision:** Added `source` field to `userAgent` object: `"platform"` | `"custom"` | `null`

**Rationale:**
- UI can show "disconnected" warning when in custom mode
- Summary panel shows platform info when linked
- Migration path for existing configs (treated as custom)

## Files Modified

| File | Change |
|------|--------|
| `launcher/platform_service.py` | New - platform/UA mapping service |
| `launcher/launcher.py` | Added API endpoints |
| `launcher/config.html` | Added platform selector UI |
| `launcher/test_platform_service.py` | New - 113 unit tests |

## Testing

Run tests:
```bash
cd F:/ChromePwn/CrossGuard/launcher
python test_platform_service.py
```

## Known Limitations

1. UA presets use Chrome 114 baseline - may become outdated with browser updates
2. Platform selector state can drift if user manually edits UA field directly
3. No support for custom platform definitions

## Future Enhancements (Out of Scope)

- Cloud-synced UA templates
- User-customizable UA presets
- Automatic UA version updates
