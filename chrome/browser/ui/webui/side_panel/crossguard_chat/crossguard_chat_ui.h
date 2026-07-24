// Copyright 2026 CrossGuard. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_SIDE_PANEL_CROSSGUARD_CHAT_CROSSGUARD_CHAT_UI_H_
#define CHROME_BROWSER_UI_WEBUI_SIDE_PANEL_CROSSGUARD_CHAT_CROSSGUARD_CHAT_UI_H_

#include "chrome/browser/ui/webui/top_chrome/top_chrome_webui_config.h"
#include "chrome/browser/ui/webui/top_chrome/untrusted_top_chrome_web_ui_controller.h"

class CrossGuardChatUI;

// 自动注册 host(chrome::kChromeUICrossGuardChatHost)→ CrossGuardChatUI。
class CrossGuardChatUIConfig
    : public DefaultTopChromeWebUIConfig<CrossGuardChatUI> {
 public:
  CrossGuardChatUIConfig();
  CrossGuardChatUIConfig(const CrossGuardChatUIConfig&) = delete;
  CrossGuardChatUIConfig& operator=(const CrossGuardChatUIConfig&) = delete;
  ~CrossGuardChatUIConfig() override;
};

// CrossGuard 原生 AI chat side panel 的 WebUI controller。
// 托管 chrome-untrusted://crossguard-chat.top-chrome/ 页面(chat.html/js)。
class CrossGuardChatUI : public UntrustedTopChromeWebUIController {
 public:
  explicit CrossGuardChatUI(content::WebUI* web_ui);
  CrossGuardChatUI(const CrossGuardChatUI&) = delete;
  CrossGuardChatUI& operator=(const CrossGuardChatUI&) = delete;
  ~CrossGuardChatUI() override;

  // WebUI 加载完成后触发 ShowUI,使 side panel content proxy available
  // (否则 WaitForEntry 不调 PopulateSidePanel,右侧 panel 不显示)。
  void WebUIPrimaryPageChanged(content::Page& page) override;

  // 已注册 "CrossGuardChatUntrusted" 到 TopChromeWebUIName 白名单
  // (tools/metrics/histograms/metadata/{ui,page}/histograms.xml)。
  static constexpr std::string GetWebUIName() { return "CrossGuardChatUntrusted"; }

  WEB_UI_CONTROLLER_TYPE_DECL();
};

#endif  // CHROME_BROWSER_UI_WEBUI_SIDE_PANEL_CROSSGUARD_CHAT_CROSSGUARD_CHAT_UI_H_
