# Database Implementation: 平台选择,对应的UA也要跟随变化

## Summary

This feature extends the existing JSON-based profile configuration system with platform selection capability.

## Changes Made

### New File: `launcher/platform_service.py`

- `PLATFORM_UA_PRESETS`: Static dictionary mapping platform IDs to full UA strings
- `PLATFORM_VALUE_MAP`: Static dictionary mapping platform IDs to Chrome platform values
- `PLATFORM_META`: Platform display metadata (id, label, icon, order)
- Helper functions:
  - `get_available_platforms()`: Returns sorted list of platforms
  - `resolve_ua_for_platform(platform_id)`: Get UA string for platform
  - `resolve_platform_value(platform_id)`: Get Chrome platform value
  - `is_valid_platform(platform_id)`: Validate platform ID
  - `apply_platform_to_config(config, platform_id)`: Apply platform to config
  - `apply_custom_ua_to_config(config, ua)`: Apply custom UA to config
  - `migrate_config(config)`: Backward-compatible config migration

### Schema Extensions

**PlatformSelector object** (embedded in profile config):
```json
{
  "type": "enum",
  "value": "windows",
  "autoUpdate": true
}
```

**userAgent object** (extended):
```json
{
  "type": 2,
  "ua": "...",
  "source": "platform"  // NEW: "platform" | "custom" | null
}
```

### No Database Migration Required

The feature uses:
- Static lookup tables in Python code (not stored in DB)
- New optional fields in existing JSON config (backward compatible)
- No schema changes to profiles.json structure

### Migration Strategy

`migrate_config()` is called when loading existing profiles:
- Adds `platformSelector` with `autoUpdate: false` for existing profiles
- Adds `source: null` to existing userAgent configs (treated as custom)
- Existing configs continue to work without modification
