# Backend Implementation: 平台选择,对应的UA也要跟随变化

## Files Modified

### `launcher/launcher.py`

**Added import:**
```python
from platform_service import (
    PLATFORM_UA_PRESETS,
    PLATFORM_VALUE_MAP,
    PLATFORM_META,
    get_available_platforms,
    resolve_ua_for_platform,
    resolve_platform_value,
    is_valid_platform,
    apply_platform_to_config,
    apply_custom_ua_to_config,
    migrate_config,
)
```

**New API endpoints:**

| Endpoint | Method | Handler | Description |
|----------|--------|---------|-------------|
| `/api/platforms` | GET | `_api_get_platforms` | Returns available platforms with metadata and UA presets |
| `/api/profile/{id}/platform` | PUT | `_api_set_platform` | Sets profile platform and auto-updates UA |
| `/api/profile/{id}/ua` | PUT | `_api_set_ua` | Sets custom UA for a profile |

**New handler methods:**
- `_api_get_platforms()`: Returns platform list with UA presets
- `_api_set_platform(pid)`: Validates platform, applies to config, saves
- `_api_set_ua(pid)`: Validates UA length (10-500), applies to config, saves

### `launcher/platform_service.py` (NEW)

Static data and helper functions for platform-to-UA mapping.

## API Details

### GET /api/platforms

**Response:**
```json
{
  "platforms": [
    {"id": "windows", "label": "Windows", "icon": "🪟", "order": 1},
    {"id": "mac", "label": "Mac", "icon": "🍎", "order": 2},
    ...
  ],
  "default": "windows",
  "uaByPlatform": {
    "windows": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36...",
    ...
  }
}
```

### PUT /api/profile/{id}/platform

**Request:**
```json
{"platform": "mac"}
```

**Response (200):**
```json
{
  "ok": true,
  "ua": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36...",
  "platformValue": "MacIntel",
  "source": "platform"
}
```

**Errors:**
- 400: `{"error": "invalid platform"}`
- 404: `{"error": "profile not found"}`

### PUT /api/profile/{id}/ua

**Request:**
```json
{"ua": "Mozilla/5.0 ...", "source": "custom"}
```

**Response (200):**
```json
{"ok": true}
```

**Errors:**
- 400: `{"error": "ua string too short or too long"}`
- 404: `{"error": "profile not found"}`

## Input Validation

- Platform ID validated against whitelist: `windows`, `mac`, `linux`, `android`, `ios`
- UA string length validated: 10-500 characters
- All input sanitized before processing
