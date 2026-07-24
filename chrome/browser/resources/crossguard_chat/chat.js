// CrossGuard 原生 chat WebUI(从扩展 content.js + bg2.js 移植)
// 通信:直接 fetch launcher(原生 WebUI 无混合内容限制,localhost 放行 + CSP connect-src)
// launcher 注入 window.__CG__ = { profileId, launcherPort }

(function () {
  "use strict";
  const CG = window.__CG__ || {};
  const BASE = CG.launcherPort ? `http://127.0.0.1:${CG.launcherPort}` : "http://127.0.0.1:18900";
  const STORAGE_KEY = "cg_chat_state_v1";

  const AGENT_TOOLS = [
    { type: "function", function: { name: "screenshot", description: "截取当前页面视口截图(你能真正看到页面内容,结果以图像返回)。用户说'截图''看看页面'时必须调用此工具。", parameters: { type: "object", properties: {} } } },
    { type: "function", function: { name: "screenshot_full", description: "截取整页滚动截图(含未滚到底部的内容,网页全部)。用户说'截全图''滚动截图''完整截图''长截图'时调用。", parameters: { type: "object", properties: {} } } },
    { type: "function", function: { name: "navigate", description: "导航当前标签页到指定 URL", parameters: { type: "object", properties: { url: { type: "string" } }, required: ["url"] } } },
    { type: "function", function: { name: "click", description: "点击匹配 CSS 选择器的元素", parameters: { type: "object", properties: { selector: { type: "string" } }, required: ["selector"] } } },
    { type: "function", function: { name: "fill", description: "在匹配 CSS 选择器的输入框填入文本(触发 input/change,兼容 React/Vue)", parameters: { type: "object", properties: { selector: { type: "string" }, value: { type: "string" } }, required: ["selector", "value"] } } },
    { type: "function", function: { name: "extract", description: "提取元素数据。mode: single|list|count。attribute 默认 textContent", parameters: { type: "object", properties: { selector: { type: "string" }, attribute: { type: "string" }, mode: { type: "string", enum: ["single", "list", "count"] } }, required: ["selector"] } } },
    { type: "function", function: { name: "get_page_text", description: "获取当前标签页的完整文本内容(DOM innerText),用于总结/分析页面。无需参数,返回截断至 32000 字符的页面文本。", parameters: { type: "object", properties: {} } } },
    { type: "function", function: { name: "inspect_dom", description: "分析页面 DOM 可交互元素(链接/按钮/输入框等),返回每个元素的 tag/id/class/text/CSS选择器/坐标。用于精确定位元素(不靠截图),决定 click/fill 的选择器。无需参数。", parameters: { type: "object", properties: {} } } },
    { type: "function", function: { name: "scroll", description: "滚动页面: selector=滚动到指定CSS选择器元素(居中), to=top滚动到顶部, to=bottom滚动到底部。", parameters: { type: "object", properties: { selector: { type: "string" }, to: { type: "string", enum: ["top", "bottom"] } } } } },
    { type: "function", function: { name: "wait", description: "等待: ms=等待毫秒(默认2000), selector=轮询等待CSS选择器元素出现(每100ms查一次,超时返回提示)。", parameters: { type: "object", properties: { ms: { type: "number" }, selector: { type: "string" } } } } },
    { type: "function", function: { name: "hover", description: "鼠标悬停到CSS选择器元素上(触发hover效果/下拉菜单)。", parameters: { type: "object", properties: { selector: { type: "string" } }, required: ["selector"] } } },
    { type: "function", function: { name: "select", description: "选择下拉框(<select>)的选项: selector=下拉框CSS选择器, value=选项value。", parameters: { type: "object", properties: { selector: { type: "string" }, value: { type: "string" } }, required: ["selector", "value"] } } },
    { type: "function", function: { name: "get_html", description: "获取CSS选择器元素的outerHTML(完整HTML结构),用于分析元素属性/结构。", parameters: { type: "object", properties: { selector: { type: "string" } }, required: ["selector"] } } },
    { type: "function", function: { name: "press_key", description: "在页面按键盘按键(key=Enter/Escape/Tab/ArrowDown等),用于确认/取消/导航。", parameters: { type: "object", properties: { key: { type: "string" } }, required: ["key"] } } },
    { type: "function", function: { name: "switch_tab", description: "切换/回到标签页: index=0回到第1个tab(首页), url=URL关键词匹配, full_url=完整URL。通过导航实现(会重新加载)。", parameters: { type: "object", properties: { index: { type: "number" }, url: { type: "string" }, full_url: { type: "string" } } } } },
    { type: "function", function: { name: "extract_table", description: "提取页面表格为JSON二维数组(行列)。selector=table选择器(默认table)。用于数据采集(如提取商品列表/价格表)。", parameters: { type: "object", properties: { selector: { type: "string" } } } } },
    { type: "function", function: { name: "new_tab", description: "新开标签页并导航到URL。用于需要多页面操作(如同时看两个网站)。", parameters: { type: "object", properties: { url: { type: "string" } }, required: ["url"] } } },
    { type: "function", function: { name: "run_js", description: "在页面执行任意 JavaScript(函数体,可 return 值)。慎用:在用户登录态下运行", parameters: { type: "object", properties: { code: { type: "string" } }, required: ["code"] } } },
  ];
  const AGENT_SYSTEM_PROMPT = "你是 CrossGuard 浏览器助手,运行在用户的环境浏览器内。你具备真实操作浏览器的能力——通过提供的工具(get_page_text/screenshot/navigate/click/fill/extract/run_js)可以抓取页面文本、截图、跳转、点击、填表、提取、执行 JS。工具真实有效,你能真正控制浏览器。\n\n工具选择规则:\n- '总结/分析/阅读当前页面内容'→**必须**调用 get_page_text(获取 DOM 文本,速度快、内容全、精度高)\n- '找元素/定位元素/页面有哪些按钮或输入框/分析页面结构'→**优先**调用 inspect_dom(精确拿选择器,不靠截图视觉)\n- '点击/填表'(需选择器)→**先** inspect_dom 拿到 sel,**再** click/fill\n- '截图/看看页面长什么样/页面布局'→调用 screenshot\n- '打开XX/跳转到XX/导航到XX'→调用 navigate\n- '搜索XX/查找XX/搜XX'→**必须在左侧tab操作**(不是AI对话框内): 先 navigate 到搜索页(如百度首页)→wait 等加载→inspect_dom 找搜索框→fill→click 搜索按钮→wait 结果→get_page_text 总结搜索结果\n- '提取/执行脚本'→相应工具\n\n- '悬停/鼠标移到XX'→调用 hover(selector=元素)\n- '选择下拉框/选XX选项'→调用 select(selector=下拉框,value=选项值)\n- '按键/回车/按Enter/Escape/Tab'→调用 press_key(key=按键名)\n- '看XX元素的HTML/结构'→调用 get_html(selector=元素)\n- '下拉/滚动到底部/翻页/滚动到顶部/移到XX元素'→调用 scroll(selector=元素/to=bottom|top)\n- '等待页面加载/等元素出现/稍等'→调用 wait\n\n流程:调工具 → 看结果 → 基于结果回复。所有操作在左侧tab进行(非AI对话框)。";

  const state = {
    models: [], modelId: null, messages: [],
    agentMode: false, profileId: CG.profileId || null,
    streaming: false, aborted: false, streamCtrl: null,
  };

  const $ = (id) => document.getElementById(id);
  const els = {
    panel: $("panel"), messages: $("messages"), model: $("model"), agent: $("agent"),
    input: $("input"), send: $("send"), new: $("new"), stop: $("stop"), hint: $("hint"),
    envBadge: $("env-badge"),
  };

  // ── 持久化(localStorage,原生 WebUI per profile 共享)──
  function saveState() {
    try {
      localStorage.setItem(STORAGE_KEY, JSON.stringify({
        messages: state.messages, modelId: state.modelId, agentMode: state.agentMode,
      }));
    } catch (e) {}
  }
  function loadState() {
    try { return JSON.parse(localStorage.getItem(STORAGE_KEY) || "{}"); }
    catch (e) { return {}; }
  }

  // ── HTTP(fetch launcher,无 background 中转)──
  async function loadModels() {
    try {
      const r = await fetch(`${BASE}/api/chat/models`);
      return await r.json();
    } catch (e) { return []; }
  }
  function renderModelSelect() {
    const sel = els.model; sel.innerHTML = "";
    if (!state.models.length) {
      const o = document.createElement("option"); o.value = ""; o.textContent = "未配置模型";
      sel.appendChild(o); sel.disabled = true; return;
    }
    sel.disabled = false;
    const ph = document.createElement("option"); ph.value = ""; ph.textContent = "选择模型…"; sel.appendChild(ph);
    for (const m of state.models) {
      const o = document.createElement("option"); o.value = String(m.id);
      o.textContent = m.name + (m.hasKey ? "" : " (无 key)"); sel.appendChild(o);
    }
    if (state.modelId != null) sel.value = String(state.modelId);
  }

  // ── 渲染 ──
  function escapeHtml(s) { return String(s).replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;"); }
  function truncate(s, n) { return s.length > n ? s.slice(0, n) + "\n[...已截断]" : s; }
  function renderMarkdown(text) {
    const parts = []; const fence = /```(\w*)\n?([\s\S]*?)```/g; let last = 0, m;
    while ((m = fence.exec(text)) !== null) {
      if (m.index > last) parts.push({ t: "p", v: text.slice(last, m.index) });
      parts.push({ t: "code", v: m[2].replace(/\n$/, "") }); last = m.index + m[0].length;
    }
    if (last < text.length) parts.push({ t: "p", v: text.slice(last) });
    return parts.map((p) => {
      if (p.t === "code") return `<pre><code>${escapeHtml(p.v)}</code></pre>`;
      let v = escapeHtml(p.v).replace(/`([^`\n]+)`/g, '<code>$1</code>');
      return "<p>" + v.split(/\n{2,}/).map((b) => b.replace(/\n/g, "<br>")).join("</p><p>") + "</p>";
    }).join("");
  }
  function extractImageUrl(content) {
    if (Array.isArray(content)) for (const p of content)
      if (p && p.type === "image_url" && p.image_url && p.image_url.url) return p.image_url.url;
    return null;
  }
  function renderMessages() {
    const box = els.messages;
    if (!state.messages.length) {
      box.innerHTML = '<div class="empty">输入问题开始对话<br>开启「Agent」可让 AI 操作当前页面(截图/搜索/填表)</div>'; return;
    }
    box.innerHTML = ""; const last = state.messages[state.messages.length - 1];
    for (const msg of state.messages) {
      const imgUrl = extractImageUrl(msg.content);
      if (msg.role === "user" && imgUrl) {
        const row = mk("div", "msg assistant"); const b = mk("div", "bubble");
        const wrap = mk("div"); wrap.style.cssText = "position:relative;display:inline-block;max-width:100%;";
        const i = document.createElement("img"); i.src = imgUrl; i.style.cssText = "max-width:100%;cursor:zoom-in;";
        i.addEventListener("click", () => showLightbox(imgUrl));
        const btn = mk("button"); btn.textContent = "🔍 1:1"; btn.style.cssText = "position:absolute;bottom:4px;right:4px;padding:2px 6px;font-size:10px;cursor:pointer;background:rgba(0,0,0,.6);color:#fff;border:none;border-radius:3px;";
        btn.addEventListener("click", (e) => { e.stopPropagation(); showLightbox(imgUrl); });
        wrap.appendChild(i); wrap.appendChild(btn); b.appendChild(wrap); row.appendChild(b); box.appendChild(row); continue;
      }
      if (msg.role === "assistant" && Array.isArray(msg.tool_calls) && msg.tool_calls.length) {
        const row = mk("div", "msg assistant"); const b = mk("div", "bubble");
        if (msg.content) b.innerHTML = renderMarkdown(msg.content);
        for (const tc of msg.tool_calls) {
          if (!tc || !tc.name) continue;
          const card = mk("div", "tool-step" + (tc.name === "run_js" ? " tool-warn" : ""));
          let args = tc.arguments || ""; try { args = JSON.stringify(JSON.parse(tc.arguments), null, 2); } catch (e) {}
          const icons = {screenshot:"📷",screenshot_full:"📷⬇",get_page_text:"📄",inspect_dom:"🔍",navigate:"🧭",click:"👆",fill:"⌨️",extract:"📊",run_js:"⚡",scroll:"📜",wait:"⏳",hover:"🖱️",select:"📋",get_html:"🏗️",press_key:"⌨️"};
          const icon = icons[tc.name] || "🔧";
          card.innerHTML = `<span class="step-icon">${icon}</span><b>${escapeHtml(tc.name)}</b>`;
          if (args && args !== "{}") card.innerHTML += ` <code>${escapeHtml(args.length > 80 ? args.slice(0,80)+"…" : args)}</code>`;
          b.appendChild(card);
        }
        row.appendChild(b); box.appendChild(row); continue;
      }
      if (msg.role === "tool") {
        const row = mk("div", "msg assistant"); const b = mk("div", "bubble tool-result");
        const ok = msg.content && !String(msg.content).match(/失败|ERR|error|timeout/i);
        const icon = ok ? "✅" : "❌";
        const text = String(msg.content || "");
        b.innerHTML = `<span class="step-result">${icon} <b>${escapeHtml(msg.name || "")}</b></span> <code>${escapeHtml(truncate(text.replace(/^[已✅]/, "").trim(), 200))}</code>`;
        row.appendChild(b); box.appendChild(row); continue;
      }
      const row = mk("div", "msg " + msg.role); const b = mk("div", "bubble" + (state.streaming && msg === last && msg.role === "assistant" ? " cursor" : ""));
      if (msg.role === "assistant") b.innerHTML = renderMarkdown(msg.content || "");
      else b.textContent = typeof msg.content === "string" ? msg.content : JSON.stringify(msg.content);
      row.appendChild(b); box.appendChild(row);
    }
    box.scrollTop = box.scrollHeight;
    addCopyButtons(box);
  }
  function mk(tag, cls) { const e = document.createElement(tag); if (cls) e.className = cls; return e; }
  function setHint(t) { els.hint.textContent = t || ""; }

  // ── 截图放大: 跳出 side panel,用左 tab 打开独立图片窗口(原尺寸)──
  function showLightbox(src) {
    // dataUrl 太长无法放 URL,改用 Blob + 临时 URL
    try {
      const base64 = src.split(",")[1];
      const mime = (src.match(/^data:(.*?);/) || [])[1] || "image/jpeg";
      const bin = atob(base64);
      const arr = new Uint8Array(bin.length);
      for (let i = 0; i < bin.length; i++) arr[i] = bin.charCodeAt(i);
      const blobUrl = URL.createObjectURL(new Blob([arr], { type: mime }));
      const w = window.open(blobUrl, "_blank");
      // 清理(避免内存泄漏,延迟避免窗口加载被中断)
      if (w) setTimeout(() => URL.revokeObjectURL(blobUrl), 60000);
    } catch (e) {
      // fallback: alert
      setHint("无法在新窗口打开截图: " + e.message);
    }
  }

  // ── Copy 按钮 ──
  function addCopyButtons(container) {
    container.querySelectorAll("pre").forEach(pre => {
      if (pre.querySelector(".cg-copy")) return;
      const btn = mk("button", "cg-copy");
      btn.textContent = "Copy";
      btn.style.cssText = "position:absolute;top:4px;right:4px;padding:2px 6px;font-size:11px;cursor:pointer;background:#444;color:#fff;border:none;border-radius:3px;opacity:0.6;";
      btn.addEventListener("mouseenter", () => btn.style.opacity = "1");
      btn.addEventListener("mouseleave", () => btn.style.opacity = "0.6");
      btn.addEventListener("click", () => {
        navigator.clipboard.writeText(pre.textContent || "").then(() => { btn.textContent = "Copied"; setTimeout(() => btn.textContent = "Copy", 1200); });
      });
      pre.style.position = "relative";
      pre.appendChild(btn);
    });
  }

  // ── 组装 payload ──
  function buildPayload() {
    const out = [];
    if (state.agentMode) out.push({ role: "system", content: AGENT_SYSTEM_PROMPT });
    // 上下文压缩:超过 30 条消息,保留最近 20 条 + 摘要前面的(防 token 爆)
    let msgs = state.messages;
    if (msgs.length > 30) {
      const dropped = msgs.slice(0, msgs.length - 20);
      const toolNames = dropped.filter(m => m.role === "tool").map(m => m.name).slice(-8);
      out.push({ role: "system", content: "[上下文已压缩] 之前 " + dropped.length + " 条消息已省略。最近工具调用: " + (toolNames.join(", ") || "无") });
      msgs = msgs.slice(-20);
    }
    const cur = state.models.find(m => String(m.id) === String(state.modelId));
    const hasVision = cur && cur.hasVision;
    for (const m of msgs) {
      if (m.role === "tool") out.push({ role: "tool", tool_call_id: m.tool_call_id, content: m.content });
      else if (m.role === "assistant") {
        const o = { role: "assistant", content: m.content || "" };
        if (Array.isArray(m.tool_calls) && m.tool_calls.length)
          o.tool_calls = m.tool_calls.filter(t => t && t.name).map(t => ({ id: t.id, type: "function", function: { name: t.name, arguments: t.arguments || "" } }));
        out.push(o);
      } else {
        if (Array.isArray(m.content) && !hasVision) {
          const txt = m.content.filter(p => p.type === "text").map(p => p.text).join(" ");
          out.push({ role: "user", content: txt || "[截图结果(当前模型不支持视觉,已省略图像)]" });
        } else out.push({ role: "user", content: m.content });
      }
    }
    return out;
  }

  // ── 单轮流式:fetch + ReadableStream 解析 SSE(原 bg2.js 逻辑,搬回前端)──
  function streamOnce(payload, round) {
    return new Promise((resolve) => {
      const assistantMsg = { role: "assistant", content: "", tool_calls: [] };
      state.messages.push(assistantMsg); renderMessages();
      const reqBody = { modelId: state.modelId, messages: payload };
      if (state.agentMode) { reqBody.tools = AGENT_TOOLS; reqBody.tool_choice = (round === 1 ? "required" : "auto"); }
      let aborted = false;
      const finish = () => { aborted = true; resolve(assistantMsg); };
      (async () => {
        try {
          const res = await fetch(`${BASE}/api/chat/completions`, {
            method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify(reqBody),
            signal: state.streamCtrl ? state.streamCtrl.signal : undefined,
          });
          if (!res.ok) { assistantMsg.content = `[错误] launcher HTTP ${res.status}`; renderMessages(); finish(); return; }
          const reader = res.body.getReader(); const dec = new TextDecoder(); let buf = "";
          while (true) {
            if (state.aborted) { reader.cancel(); finish(); return; }
            const { value, done } = await reader.read(); if (done) break;
            buf += dec.decode(value, { stream: true });
            let idx;
            while ((idx = buf.indexOf("\n\n")) >= 0) {
              const raw = buf.slice(0, idx); buf = buf.slice(idx + 2); const line = raw.trim();
              if (!line.startsWith("data:")) continue;
              const d = line.slice(5).trim(); if (d === "[DONE]") { finish(); return; }
              try {
                const obj = JSON.parse(d); const ch = obj.choices && obj.choices[0];
                const delta = ch && (ch.delta || ch.message);
                if (delta) {
                  if (delta.content) { assistantMsg.content += delta.content; renderMessages(); }
                  if (Array.isArray(delta.tool_calls)) {
                    for (const tc of delta.tool_calls) {
                      const i = tc.index || 0;
                      while (assistantMsg.tool_calls.length <= i) assistantMsg.tool_calls.push({ id: "", name: "", arguments: "" });
                      const sl = assistantMsg.tool_calls[i];
                      if (tc.id) sl.id = tc.id;
                      if (tc.function) { if (tc.function.name) sl.name = tc.function.name; if (tc.function.arguments) sl.arguments += tc.function.arguments; }
                    }
                    renderMessages();
                  }
                }
                if (obj.error) { assistantMsg.content += (assistantMsg.content ? "\n\n" : "") + "[错误] " + (typeof obj.error === "string" ? obj.error : JSON.stringify(obj.error)); renderMessages(); }
              } catch (e) {}
            }
          }
          finish();
        } catch (e) { assistantMsg.content = `[错误] ${e}`; renderMessages(); finish(); }
      })();
    });
  }

  // ── 执行工具(直 fetch launcher /api/agent/*)──
  async function execTool(tc) {
    if (state.aborted) return { kind: "text", content: "[已停止]" };
    let args = {}; try { args = JSON.parse(tc.arguments || "{}"); } catch (e) {}
    if (tc.name === "screenshot" || tc.name === "screenshot_full") {
      if (state.aborted) return { kind: "text", content: "[已停止]" };
      const isFull = tc.name === "screenshot_full";
      let resp = null;
      for (let i = 0; i < 4; i++) {
        if (state.aborted) return { kind: "text", content: "[已停止]" };
        try {
          const ctrl = new AbortController();
          const t = setTimeout(() => ctrl.abort(), isFull ? 25000 : 15000);
          const r = await fetch(`${BASE}/api/agent/screenshot`, { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify({ profileId: state.profileId, full: isFull }), signal: ctrl.signal });
          clearTimeout(t);
          resp = await r.json();
        } catch (e) {}
        if (resp && resp.ok && resp.dataUrl) break;
        await new Promise((r) => setTimeout(r, 800));
      }
      if (state.aborted) return { kind: "text", content: "[已停止]" };
      if (resp && resp.ok && resp.dataUrl) {
        const cur = state.models.find(x => String(x.id) === String(state.modelId));
        if (cur && cur.hasVision) return { kind: "screenshot", content: "已截图,见下一消息", image: resp.dataUrl };
        return { kind: "text", content: "[截图完成] 当前模型不支持视觉识别，截图 dataUrl 长度: " + resp.dataUrl.length + "。请切换到支持视觉的模型(如 GLM)，然后发送'总结当前页面'。" };
      }
      return { kind: "text", content: "[screenshot 失败] " + ((resp && resp.error) || "未知") };
    }
    if (state.aborted) return { kind: "text", content: "[已停止]" };
    // get_page_text:直连 launcher 专用端点(CDP Runtime.evaluate 抓 innerText)
    if (tc.name === "get_page_text") {
      if (state.aborted) return { kind: "text", content: "[已停止]" };
      let resp = null;
      try {
        const ctrl = new AbortController();
        const t = setTimeout(() => ctrl.abort(), 15000);
        const r = await fetch(`${BASE}/api/agent/page-text`, { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify({ profileId: state.profileId }), signal: ctrl.signal });
        clearTimeout(t);
        resp = await r.json();
      } catch (e) { resp = { _err: e.name === 'AbortError' ? '超时(15s)' : String(e) }; }
      if (resp && resp.ok) return { kind: "text", content: resp.text ? truncate(resp.text, 32000) : "(页面无文本)" };
      return { kind: "text", content: "[获取页面文本失败] " + ((resp && (resp.error || resp._err)) || "未知") };
    }
    if (tc.name === "inspect_dom") {
      if (state.aborted) return { kind: "text", content: "[已停止]" };
      let resp = null;
      try {
        const ctrl = new AbortController();
        const t = setTimeout(() => ctrl.abort(), 15000);
        const r = await fetch(`${BASE}/api/agent/inspect-dom`, { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify({ profileId: state.profileId }), signal: ctrl.signal });
        clearTimeout(t);
        resp = await r.json();
      } catch (e) { resp = { _err: e.name === 'AbortError' ? '超时(15s)' : String(e) }; }
      if (resp && resp.ok) return { kind: "text", content: "DOM 元素列表:\n" + JSON.stringify(resp.elements, null, 2).slice(0, 32000) };
      return { kind: "text", content: "[DOM 分析失败] " + ((resp && (resp.error || resp._err)) || "未知") };
    }
    if (state.aborted) return { kind: "text", content: "[已停止]" };
    let resp = null;
    try {
      const ctrl = new AbortController();
      const t = setTimeout(() => ctrl.abort(), 20000);
      const r = await fetch(`${BASE}/api/agent/tool`, { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify({ profileId: state.profileId, action: tc.name, params: args }), signal: ctrl.signal });
      clearTimeout(t);
      resp = await r.json();
    } catch (e) { resp = { _err: e.name === 'AbortError' ? '工具超时(20s)' : String(e) }; }
    if (resp && resp.ok) return { kind: "text", content: truncate(JSON.stringify({ log: resp.log, outputs: resp.outputs }), 8192) };
    return { kind: "text", content: "[工具失败] " + ((resp && (resp.error || resp._err)) || "未知") };
  }

  // ── Agent 循环 ──
  async function runConversation() {
    state.streaming = true; state.aborted = false; state.streamCtrl = new AbortController();
    els.send.disabled = true; els.stop.classList.remove("hidden");
    const MAX = 8; let round = 0;
    while (round < MAX && !state.aborted) {
      round++;
      const payload = buildPayload(); const asst = await streamOnce(payload, round);
      if (state.aborted) break;
      const tcs = (asst.tool_calls || []).filter(t => t && t.name);
      if (!tcs.length) break;
      for (const tc of tcs) {
        if (state.aborted) break;
        const r = await execTool(tc);
        state.messages.push({ role: "tool", tool_call_id: tc.id, name: tc.name, content: r.content });
        if (r.kind === "screenshot" && r.image)
          state.messages.push({ role: "user", content: [{ type: "text", text: "📸 screenshot 结果:" }, { type: "image_url", image_url: { url: r.image } }] });
        renderMessages();
      }
    }
    if (round >= MAX && !state.aborted) state.messages.push({ role: "assistant", content: "(已达 Agent 最大轮数 8,停止。)" });
    state.streaming = false; els.send.disabled = false; els.stop.classList.add("hidden");
    renderMessages(); saveState();
  }

  function send() {
    if (state.streaming) return;
    const text = els.input.value.trim(); if (!text) return;
    const model = state.models.find(m => String(m.id) === String(state.modelId));
    if (!model) { setHint("请先选择模型(管理界面「Chat 模型」配置)"); return; }
    if (!model.hasKey) { setHint("该模型未配置 API key"); return; }
    if (state.agentMode && !state.profileId) { setHint("Agent 需要 profileId(未从 launcher 注入)"); return; }
    setHint(""); els.input.value = ""; autoSize();
    state.messages.push({ role: "user", content: text }); renderMessages();
    runConversation();
  }
  function newChat() { if (state.streaming) return; state.messages = []; setHint(""); renderMessages(); saveState(); }
  function autoSize() { const t = els.input; t.style.height = "auto"; t.style.height = Math.min(t.scrollHeight, 120) + "px"; }

  // ── 事件 ──
  els.send.addEventListener("click", send);
  els.new.addEventListener("click", newChat);
  els.stop.addEventListener("click", () => { state.aborted = true; if (state.streamCtrl) { try { state.streamCtrl.abort(); } catch (e) {} } setHint("正在停止…"); });
  els.model.addEventListener("change", () => { state.modelId = els.model.value ? parseInt(els.model.value, 10) : null; saveState(); });
  els.agent.addEventListener("change", () => { state.agentMode = els.agent.checked; saveState(); });
  els.input.addEventListener("input", autoSize);
  els.input.addEventListener("keydown", (e) => { if ((e.ctrlKey || e.metaKey) && e.key === "Enter") { e.preventDefault(); send(); } });

  // ── 初始化 ──
  (async function init() {
    if (els.envBadge) els.envBadge.textContent = state.profileId ? `环境 #${state.profileId}` : "环境 -";
    const saved = loadState();
    state.messages = Array.isArray(saved.messages) ? saved.messages : [];
    state.modelId = saved.modelId != null ? saved.modelId : null;
    state.agentMode = !!saved.agentMode;
    els.agent.checked = state.agentMode;
    renderMessages();
    state.models = await loadModels();
    if (state.modelId != null && !state.models.some(m => m.id === state.modelId)) state.modelId = null;
    renderModelSelect();
    if (state.modelId != null) els.model.value = String(state.modelId);
  })();
})();
