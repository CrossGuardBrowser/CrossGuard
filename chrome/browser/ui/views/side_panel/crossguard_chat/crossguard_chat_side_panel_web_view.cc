// Copyright 2026 CrossGuard. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "chrome/browser/ui/views/side_panel/crossguard_chat/crossguard_chat_side_panel_web_view.h"

#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/webui/top_chrome/webui_contents_wrapper.h"
#include "chrome/common/webui_url_constants.h"
#include "chrome/grit/generated_resources.h"
#include "ui/base/metadata/metadata_impl_macros.h"

using SidePanelWebUIViewT_CrossGuardChatUI =
    SidePanelWebUIViewT<CrossGuardChatUI>;
BEGIN_TEMPLATE_METADATA(SidePanelWebUIViewT_CrossGuardChatUI,
                        SidePanelWebUIViewT);
END_METADATA

CrossGuardChatSidePanelWebView::CrossGuardChatSidePanelWebView(
    Profile* profile,
    SidePanelEntryScope& scope)
    : SidePanelWebUIViewT(
          scope,
          base::RepeatingClosure(),
          base::RepeatingClosure(),
          std::make_unique<WebUIContentsWrapperT<CrossGuardChatUI>>(
              GURL(chrome::kChromeUICrossGuardChatURL),
              profile,
              IDS_READING_MODE_TITLE,
              /*esc_closes_ui=*/false)) {}

CrossGuardChatSidePanelWebView::~CrossGuardChatSidePanelWebView() = default;

BEGIN_METADATA(CrossGuardChatSidePanelWebView)
END_METADATA
