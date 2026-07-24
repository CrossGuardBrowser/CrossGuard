// Copyright 2026 CrossGuard. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_SIDE_PANEL_CROSSGUARD_CHAT_CROSSGUARD_CHAT_SIDE_PANEL_COORDINATOR_H_
#define CHROME_BROWSER_UI_VIEWS_SIDE_PANEL_CROSSGUARD_CHAT_CROSSGUARD_CHAT_SIDE_PANEL_COORDINATOR_H_

#include "chrome/browser/ui/browser_user_data.h"

class Browser;
class SidePanelEntryScope;
class SidePanelRegistry;

namespace views {
class View;
}  // namespace views

// 创建并注册 CrossGuard AI chat 的 SidePanelEntry(全局,所有 tab 可用)。
class CrossGuardChatSidePanelCoordinator
    : public BrowserUserData<CrossGuardChatSidePanelCoordinator> {
 public:
  explicit CrossGuardChatSidePanelCoordinator(Browser* browser);
  ~CrossGuardChatSidePanelCoordinator() override;

  void CreateAndRegisterEntry(SidePanelRegistry* global_registry);

 private:
  friend class BrowserUserData<CrossGuardChatSidePanelCoordinator>;
  std::unique_ptr<views::View> CreateChatWebView(SidePanelEntryScope& scope);
  BROWSER_USER_DATA_KEY_DECL();
};

#endif  // CHROME_BROWSER_UI_VIEWS_SIDE_PANEL_CROSSGUARD_CHAT_CROSSGUARD_CHAT_SIDE_PANEL_COORDINATOR_H_
