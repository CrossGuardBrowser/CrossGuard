// Copyright 2026 CrossGuard. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "chrome/browser/ui/views/side_panel/crossguard_chat/crossguard_chat_side_panel_coordinator.h"

#include <memory>

#include "base/functional/callback.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/views/side_panel/crossguard_chat/crossguard_chat_side_panel_web_view.h"
#include "chrome/browser/ui/views/side_panel/side_panel_entry.h"
#include "chrome/browser/ui/views/side_panel/side_panel_registry.h"

CrossGuardChatSidePanelCoordinator::CrossGuardChatSidePanelCoordinator(
    Browser* browser)
    : BrowserUserData<CrossGuardChatSidePanelCoordinator>(*browser) {}

CrossGuardChatSidePanelCoordinator::~CrossGuardChatSidePanelCoordinator() =
    default;

void CrossGuardChatSidePanelCoordinator::CreateAndRegisterEntry(
    SidePanelRegistry* global_registry) {
  auto entry = std::make_unique<SidePanelEntry>(
      SidePanelEntry::Key(SidePanelEntry::Id::kCrossGuardChat),
      base::BindRepeating(
          &CrossGuardChatSidePanelCoordinator::CreateChatWebView,
          base::Unretained(this)),
      SidePanelEntry::kSidePanelDefaultContentWidth);
  // chat.html 自带 header("AI 对话"),隐藏 side panel 系统标题栏,
  // 否则顶部会显示 action item 的 title(原为"朗读模式")。
  entry->SetProperty(kShouldShowTitleInSidePanelHeaderKey, false);
  global_registry->Register(std::move(entry));
}

std::unique_ptr<views::View>
CrossGuardChatSidePanelCoordinator::CreateChatWebView(
    SidePanelEntryScope& scope) {
  return std::make_unique<CrossGuardChatSidePanelWebView>(
      Profile::FromBrowserContext(GetBrowser().profile()), scope);
}

BROWSER_USER_DATA_KEY_IMPL(CrossGuardChatSidePanelCoordinator);
