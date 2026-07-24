// Copyright 2026 CrossGuard. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_SIDE_PANEL_CROSSGUARD_CHAT_CROSSGUARD_CHAT_SIDE_PANEL_WEB_VIEW_H_
#define CHROME_BROWSER_UI_VIEWS_SIDE_PANEL_CROSSGUARD_CHAT_CROSSGUARD_CHAT_SIDE_PANEL_WEB_VIEW_H_

#include "chrome/browser/ui/views/side_panel/side_panel_web_ui_view.h"
#include "chrome/browser/ui/webui/side_panel/crossguard_chat/crossguard_chat_ui.h"

// 托管 chrome-untrusted://crossguard-chat.top-chrome/ 的 WebContents。
class CrossGuardChatSidePanelWebView
    : public SidePanelWebUIViewT<CrossGuardChatUI> {
  using SidePanelWebUIViewT_CrossGuardChatUI =
      SidePanelWebUIViewT<CrossGuardChatUI>;
  METADATA_HEADER(CrossGuardChatSidePanelWebView,
                  SidePanelWebUIViewT_CrossGuardChatUI)

 public:
  CrossGuardChatSidePanelWebView(Profile* profile, SidePanelEntryScope& scope);
  CrossGuardChatSidePanelWebView(const CrossGuardChatSidePanelWebView&) = delete;
  CrossGuardChatSidePanelWebView& operator=(
      const CrossGuardChatSidePanelWebView&) = delete;
  ~CrossGuardChatSidePanelWebView() override;
};

#endif  // CHROME_BROWSER_UI_VIEWS_SIDE_PANEL_CROSSGUARD_CHAT_CROSSGUARD_CHAT_SIDE_PANEL_WEB_VIEW_H_
