// Copyright 2026 CrossGuard. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "chrome/browser/ui/webui/side_panel/crossguard_chat/crossguard_chat_ui.h"

#include "base/command_line.h"
#include "cc/base/switches.h"
#include "chrome/common/webui_url_constants.h"
#include "chrome/grit/crossguard_chat_resources.h"
#include "content/public/browser/page.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/url_constants.h"
#include "content/public/browser/web_ui_data_source.h"
#include "services/network/public/mojom/content_security_policy.mojom-shared.h"

CrossGuardChatUIConfig::CrossGuardChatUIConfig()
    : DefaultTopChromeWebUIConfig(content::kChromeUIUntrustedScheme,
                                  chrome::kChromeUICrossGuardChatHost) {}
CrossGuardChatUIConfig::~CrossGuardChatUIConfig() = default;

CrossGuardChatUI::CrossGuardChatUI(content::WebUI* web_ui)
    : UntrustedTopChromeWebUIController(web_ui) {
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      web_ui->GetWebContents()->GetBrowserContext(),
      chrome::kChromeUICrossGuardChatURL);
  source->UseStringsJs();
  source->EnableReplaceI18nInJS();
  source->AddResourcePath("chat.html", IDR_CROSSGUARD_CHAT_CHAT_HTML);
  source->AddResourcePath("chat.css", IDR_CROSSGUARD_CHAT_CHAT_CSS);
  source->AddResourcePath("chat.js", IDR_CROSSGUARD_CHAT_CHAT_JS);
  source->AddResourcePath("", IDR_CROSSGUARD_CHAT_CHAT_HTML);
  source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ScriptSrc,
      "script-src 'self' 'unsafe-inline';");
  source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ConnectSrc,
      "connect-src 'self' http://127.0.0.1:* http://localhost:*;");
  // WebUIDataSource 默认 img-src 不含 data:,截图 dataUrl 无法渲染。
  source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ImgSrc,
      "img-src 'self' data:;");
  // WebUIDataSource 默认启用 require-trusted-types-for 'script',
  // 阻止 chat.js 的 raw innerHTML(非 TrustedHTML)→ init 抛错中断
  // → loadModels/renderModelSelect 没执行→模型下拉空。
  source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::RequireTrustedTypesFor,
      "");
  auto* cmd = base::CommandLine::ForCurrentProcess();
  source->AddString(
      "profileId",
      cmd->GetSwitchValueASCII(switches::kCrossGuardProfileId));
  source->AddString(
      "launcherPort",
      cmd->GetSwitchValueASCII(switches::kCrossGuardLauncherPort));
}

CrossGuardChatUI::~CrossGuardChatUI() = default;

void CrossGuardChatUI::WebUIPrimaryPageChanged(content::Page& page) {
  UntrustedTopChromeWebUIController::WebUIPrimaryPageChanged(page);
  // page 加载完成 → 通知 embedder 显示 UI,触发 side panel content proxy
  // available,使 WaitForEntry → PopulateSidePanel 真正展开 panel。
  if (embedder()) {
    embedder()->ShowUI();
  }
}

WEB_UI_CONTROLLER_TYPE_IMPL(CrossGuardChatUI)
