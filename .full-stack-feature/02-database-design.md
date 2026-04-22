# Database Design: 平台选择,对应的UA也要跟随变化

## 1. Entity Relationship Design

### Core Entities

| Entity | Type | Description |
|--------|------|-------------|
| `Profile` | Parent | Container for one browser environment configuration |
| `FingerprintConfig` | Child | Contains all fingerprint settings for a profile |
| `PlatformSelector` | Embedded | Platform-to-UA mapping lookup within config |
| `UserAgentConfig` | Embedded | User-Agent string configuration |
| `PlatformConfig` | Embedded | Platform/OS identifier configuration |

### Relationships

```
Profile (1) ────── (1) FingerprintConfig
                           │
                           ├── (1) UserAgentConfig
                           ├── (1) PlatformConfig
                           └── (1) PlatformSelector  ← NEW
```

---

## 2. Schema Definitions

### 2.1 PlatformSelector (New - Static Lookup Table)

```json
{
  "_comment": "Static lookup table, embedded in launcher code",
  "platforms": {
    "windows": {
      "id": "windows",
      "displayName": "Windows",
      "ua": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.5735.199 Safari/537.36",
      "platformValue": "Win32",
      "order": 1
    },
    "mac": {
      "id": "mac",
      "displayName": "Mac",
      "ua": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.5735.199 Safari/537.36",
      "platformValue": "MacIntel",
      "order": 2
    },
    "linux": {
      "id": "linux",
      "displayName": "Linux",
      "ua": "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.5735.199 Safari/537.36",
      "platformValue": "Linux x86_64",
      "order": 3
    },
    "android": {
      "id": "android",
      "displayName": "Android",
      "ua": "Mozilla/5.0 (Linux; Android 13; Pixel 7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.5735.199 Mobile Safari/537.36",
      "platformValue": "Linux armv8l",
      "order": 4
    },
    "ios": {
      "id": "ios",
      "displayName": "iOS",
      "ua": "Mozilla/5.0 (iPhone; CPU iPhone OS 16_6 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/16.6 Mobile/15E148 Safari/604.1",
      "platformValue": "iPhone",
      "order": 5
    }
  }
}
```

### 2.2 Extended FingerprintConfig (Changes to existing structure)

```json
{
  "init": 2,

  "platformSelector": {
    "type": "enum",
    "value": "windows",
    "autoUpdate": true
  },

  "userAgent": {
    "type": 2,
    "ua": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.5735.199 Safari/537.36",
    "source": "platform"  // NEW: "platform" | "custom" | "preset"
  },

  "platform": {
    "type": 0,
    "value": "Win32"
  },

  // ... rest of existing config unchanged
}
```

---

## 3. Field Specifications

### PlatformSelector Object

| Field | Type | Required | Default | Description |
|-------|------|----------|---------|-------------|
| `type` | string | Yes | `"enum"` | Selector type (only `enum` for now) |
| `value` | string | Yes | `"windows"` | Selected platform ID |
| `autoUpdate` | boolean | Yes | `true` | Whether to auto-sync UA when platform changes |

### UserAgentConfig (Modified)

| Field | Type | Required | Default | Description |
|-------|------|----------|---------|-------------|
| `type` | integer | Yes | `2` | 0=real, 1=preset, 2=custom |
| `ua` | string | Yes | - | User-Agent string |
| `source` | string | No | `null` | `"platform"` if auto-generated, `"custom"` if user-entered |

### PlatformConfig (Unchanged)

| Field | Type | Required | Default | Description |
|-------|------|----------|---------|-------------|
| `type` | integer | Yes | `0` | 0=real, 2=custom |
| `value` | string | Yes | - | Platform string (e.g., `"Win32"`) |

---

## 4. Indexing Strategy

JSON file-based store - no traditional indexing needed. Profile count expected <100.

| Query Pattern | Optimization |
|--------------|--------------|
| Find profile by ID | `profiles[].id` - O(n) array scan |
| List all profiles | Return all - no indexing needed |
| Save profile | Direct write by ID |

---

## 5. Migration Strategy

### Phase 1: Backward-Compatible Extension

```python
def migrate_config(config):
    """Add platformSelector to existing config without breaking changes."""

    if "platformSelector" not in config:
        config["platformSelector"] = {
            "type": "enum",
            "value": _detect_platform_from_ua(config.get("userAgent", {}).get("ua", "")),
            "autoUpdate": False  # Default to false for existing configs
        }

    if "source" not in config.get("userAgent", {}):
        config["userAgent"]["source"] = None  # null = legacy/custom

    return config
```

### Rollback Plan

- Keep `platformSelector.autoUpdate = false` for existing profiles
- Existing UA/platform values preserved
- Only new profiles get `autoUpdate = true`

---

## 6. Query Patterns

### Read Operations

| Operation | Input | Output |
|-----------|-------|--------|
| Get platform list | None | List of all platforms with displayName and order |
| Get selected platform | profile_id | Current `platformSelector.value` |
| Get effective UA | profile_id | `userAgent.ua` |
| Get all profiles | None | Array of profile metadata |

### Write Operations

| Operation | Input | Behavior |
|-----------|-------|----------|
| Select platform | profile_id, platform_id | If `autoUpdate=true`: update both `userAgent.ua` and `platform.value` |
| Manual UA edit | profile_id, ua_string | Set `userAgent.ua`, set `userAgent.source = "custom"`, set `platformSelector.autoUpdate = false` |
| Toggle autoUpdate | profile_id, boolean | Enable/disable auto-sync |

---

## 7. Data Access Layer

```python
# launcher/platform_selector.py

class PlatformSelectorRepository:
    PLATFORMS = {
        "windows": PlatformDef(
            id="windows",
            displayName="Windows",
            ua="Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
               "(KHTML, like Gecko) Chrome/114.0.5735.199 Safari/537.36",
            platformValue="Win32",
            order=1
        ),
        # ... other platforms
    }

    def get_available_platforms(self) -> list[PlatformDef]:
        """Return all platforms sorted by order."""
        return sorted(self.PLATFORMS.values(), key=lambda p: p.order)

    def get_platform_ua(self, platform_id: str) -> str:
        """Get UA string for a platform."""
        return self.PLATFORMS.get(platform_id).ua

    def get_platform_value(self, platform_id: str) -> str:
        """Get platform value string for a platform."""
        return self.PLATFORMS.get(platform_id).platformValue

    def sync_ua_from_platform(self, config: dict, platform_id: str) -> dict:
        """Update userAgent and platform from selected platform."""
        platform = self.PLATFORMS.get(platform_id)
        if not platform:
            return config

        config["userAgent"]["ua"] = platform.ua
        config["userAgent"]["source"] = "platform"
        config["platform"]["value"] = platform.platformValue

        return config
```

---

## 8. API Extension

| Endpoint | Method | Request | Response |
|----------|--------|---------|----------|
| `/api/platforms` | GET | - | `{"platforms": [...], "default": "windows"}` |
| `/api/profile/{id}/platform` | PUT | `{"platform": "mac"}` | `{"ok": true, "ua": "...", "platformValue": "MacIntel"}` |
| `/api/profile/{id}/ua` | PUT | `{"ua": "...", "source": "custom"}` | `{"ok": true}` |
