/* CrossGuard 站点交互 */
(function () {
  "use strict";

  var nav = document.getElementById("nav");
  var navToggle = document.getElementById("navToggle");
  var navLinks = document.getElementById("navLinks");
  var progress = document.getElementById("scrollProgress");

  /* ---- 滚动进度条 + 导航栏背景 ---- */
  function onScroll() {
    var st = window.scrollY || document.documentElement.scrollTop;
    var h = document.documentElement.scrollHeight - window.innerHeight;
    progress.style.width = (h > 0 ? (st / h) * 100 : 0) + "%";
    if (st > 20) nav.classList.add("scrolled");
    else nav.classList.remove("scrolled");
    highlightNav();
  }
  window.addEventListener("scroll", onScroll, { passive: true });

  /* ---- 移动端菜单 ---- */
  navToggle.addEventListener("click", function () {
    var open = navLinks.classList.toggle("open");
    navToggle.classList.toggle("open", open);
    navToggle.setAttribute("aria-expanded", open ? "true" : "false");
  });
  // 点击菜单项后自动收起
  navLinks.querySelectorAll("a").forEach(function (a) {
    a.addEventListener("click", function () {
      if (navLinks.classList.contains("open")) {
        navLinks.classList.remove("open");
        navToggle.classList.remove("open");
        navToggle.setAttribute("aria-expanded", "false");
      }
    });
  });

  /* ---- 滚动显现 (IntersectionObserver) ---- */
  var reveals = document.querySelectorAll(".reveal");
  if ("IntersectionObserver" in window) {
    var io = new IntersectionObserver(function (entries) {
      entries.forEach(function (e) {
        if (e.isIntersecting) {
          // 同一容器内的元素稍微错开，更有节奏
          var el = e.target;
          var siblings = Array.prototype.slice.call(el.parentElement.children).filter(function (c) { return c.classList.contains("reveal"); });
          var idx = siblings.indexOf(el);
          el.style.transitionDelay = Math.min(idx * 60, 240) + "ms";
          el.classList.add("in");
          io.unobserve(el);
        }
      });
    }, { threshold: 0.12, rootMargin: "0px 0px -40px 0px" });
    reveals.forEach(function (el) { io.observe(el); });
  } else {
    reveals.forEach(function (el) { el.classList.add("in"); });
  }

  /* ---- 数字滚动动画 ---- */
  function animateCounter(el) {
    var target = parseInt(el.getAttribute("data-target"), 10) || 0;
    var suffix = el.getAttribute("data-suffix") || "";
    var dur = 1400, start = 0, t0 = null;
    function step(ts) {
      if (!t0) t0 = ts;
      var p = Math.min((ts - t0) / dur, 1);
      // easeOutExpo
      var eased = p === 1 ? 1 : 1 - Math.pow(2, -10 * p);
      el.textContent = Math.round(eased * (target - start) + start) + suffix;
      if (p < 1) requestAnimationFrame(step);
    }
    requestAnimationFrame(step);
  }
  var counters = document.querySelectorAll(".stat-num");
  if ("IntersectionObserver" in window) {
    var cio = new IntersectionObserver(function (entries) {
      entries.forEach(function (e) {
        if (e.isIntersecting) { animateCounter(e.target); cio.unobserve(e.target); }
      });
    }, { threshold: 0.5 });
    counters.forEach(function (el) { cio.observe(el); });
  } else {
    counters.forEach(animateCounter);
  }

  /* ---- 导航高亮当前章节 ---- */
  var sections = Array.prototype.slice.call(document.querySelectorAll("section[id]"));
  var linkMap = {};
  navLinks.querySelectorAll(".nav-link").forEach(function (l) {
    var href = l.getAttribute("href");
    if (href && href.charAt(0) === "#" && href.length > 1) linkMap[href.slice(1)] = l;
  });
  function highlightNav() {
    var pos = (window.scrollY || document.documentElement.scrollTop) + 120;
    var current = "";
    for (var i = 0; i < sections.length; i++) {
      if (sections[i].offsetTop <= pos) current = sections[i].id;
    }
    Object.keys(linkMap).forEach(function (id) {
      linkMap[id].classList.toggle("active", id === current);
    });
  }

  /* ---- FAQ: 单一展开(可选),这里允许多开,仅做平滑 ---- */
  document.querySelectorAll(".faq-item").forEach(function (item) {
    item.addEventListener("toggle", function () {});
  });

  /* ---- 页脚年份 ---- */
  var y = document.getElementById("year");
  if (y) y.textContent = new Date().getFullYear();

  onScroll();
})();
