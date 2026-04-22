# Deployment: 平台选择,对应的UA也要跟随变化

## Deployment Summary

This feature is deployed as part of the CrossGuard launcher package.

## What Gets Deployed

### Backend Changes
- `launcher/platform_service.py` - New platform service module
- `launcher/launcher.py` - Modified with new API endpoints

### Frontend Changes
- `launcher/config.html` - Modified with platform selector UI

## Deployment Process

The feature is deployed via the existing CrossGuard build pipeline:

```bash
cd F:/ChromePwn/CrossGuard/launcher
python build.py --chrome-dir "F:/ChromePwn/chromium/src/out/CrossGuard"
```

This automatically:
1. Packages launcher.py with PyInstaller → CrossGuard.exe
2. Copies config.html with platform selector to package
3. Copies Chromium runtime files
4. Generates Inno Setup installer

## Files Included in Installer

| File | Description |
|------|-------------|
| `CrossGuard.exe` | Launcher with platform selector API |
| `config.html` | Web UI with platform selector |
| `chrome/*` | Chromium 138 runtime |
| `logo.png` | Application icon |

## New API Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/platforms` | GET | Returns platform list and UA presets |
| `/api/profile/{id}/platform` | PUT | Sets platform and auto-updates UA |
| `/api/profile/{id}/ua` | PUT | Sets custom UA |

## No New Infrastructure Required

- Uses existing JSON file storage (profiles.json)
- No database migrations
- No external services
- Localhost-only HTTP API

## Rollback Procedure

To rollback:
1. Remove `platform_service.py`
2. Revert `launcher.py` to previous version
3. Revert `config.html` to previous version
4. Rebuild the installer

## Monitoring

No new monitoring required. Existing logs cover new endpoints:
- `[配置]` prefix logs for platform/UA changes
- Standard HTTP request logs
