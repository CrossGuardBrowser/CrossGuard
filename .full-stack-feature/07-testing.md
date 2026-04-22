# Testing & Validation: 平台选择,对应的UA也要跟随变化

## Test Suite

**File Created:** `F:\ChromePwn\CrossGuard\launcher\test_platform_service.py`

All 113 tests pass.

| Category | Tests | Description |
|----------|-------|-------------|
| `TestPlatformUaPresets` | 8 | UA preset constants validation |
| `TestPlatformValueMap` | 5 | Chrome platform value mapping |
| `TestPlatformMeta` | 3 | Platform metadata structure |
| `TestValidPlatforms` | 2 | VALID_PLATFORMS constant |
| `TestGetAvailablePlatforms` | 5 | `get_available_platforms()` |
| `TestResolveUaForPlatform` | 8 | `resolve_ua_for_platform()` including defaults |
| `TestResolvePlatformValue` | 7 | `resolve_platform_value()` including defaults |
| `TestIsValidPlatform` | 10 | Platform validation, case sensitivity, edge cases |
| `TestApplyPlatformToConfig` | 7 | Full platform application to config |
| `TestApplyCustomUaToConfig` | 5 | Custom UA application and autoUpdate handling |
| `TestMigrateConfig` | 10 | Migration logic including all platform detection paths |
| `TestDetectPlatformFromUa` | 10 | UA detection for all 5 platforms + edge cases |
| `TestApiGetPlatforms` | 3 | GET /api/platforms handler |
| `TestApiSetPlatform` | 3 | PUT /api/profile/{id}/platform handler |
| `TestApiSetUa` | 6 | PUT /api/profile/{id}/ua handler including boundary validation |
| `TestEdgeCases` | 7 | None handling, type safety, consistency checks |
| `TestNormalizeConfigMigration` | 6 | Frontend migration logic simulation |

**Run with:** `python test_platform_service.py`

---

## Security Findings

### HIGH Severity

**1. XSS in Proxy Host/Port Summary Display** (config.html:1002-1003)
- Existing code, not part of this feature
- Proxy host/port inserted into innerHTML without escaping
- Recommendation: Wrap with `esc()` - noted but not fixed as part of this feature

### MEDIUM Severity

1. **Profile Name onclick Handler** (config.html:626) - Existing code pattern issue
2. **Source Parameter Validation Missing** (launcher.py:793) - No whitelist validation for source field
3. **No Profile ID Ownership Check in do_PUT** - localhost-only service, low risk
4. **Content-Length No Upper Bound** - Could add 1MB limit, low risk for this feature

### LOW Severity

1. List field display via innerHTML (existing pattern)
2. Hint text via innerHTML (static strings only)
3. Placeholder text consistency

### Positive Security Findings

- Platform ID properly validated against whitelist
- UA length validated (10-500 chars)
- Esc function correctly implemented using textContent approach
- No path traversal, SQL injection, or SSRF risks in new code

---

## Performance Findings

### MEDIUM Severity

**1. Full DOM Row Replacements on Sync** (syncPlatformToConfig)
- `syncPlatformToConfig()` replaces entire module rows on each platform sync
- Causes multiple reflows/repaints
- Recommendation: Targeted DOM updates for changed fields only

### LOW Severity

1. **Blocking Fetch on editProfile** - Could inline static data (5 entries, ~1KB)
2. **Repeated load_profiles() Calls** - Could add in-memory cache
3. **Memory for platformPresets** - Negligible (~1KB)
4. **Config Normalization** - O(1), no concerns

---

## Action Items

### Before Delivery (Optional - Not in Feature Scope)

1. [ ] Fix XSS in proxy host display (existing code, outside feature scope)
2. [ ] Add source parameter validation to `launcher.py`
3. [ ] Add Content-Length bounds check (1MB limit)

### Nice-to-have Optimizations

4. [ ] Refactor `syncPlatformToConfig()` to use targeted DOM updates
5. [ ] Consider inlining static platform data in frontend JS
