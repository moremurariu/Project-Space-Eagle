// ==UserScript==
// @name         Beach Please Drop Assist (eMAG)
// @namespace    project-space-eagle
// @version      1.0.0
// @description  eMAG-server-synced countdown, latency-compensated reload at 08:00:00, and glowing next-button guidance through checkout. You do every click.
// @match        https://www.emag.ro/*
// @run-at       document-idle
// @grant        none
// ==/UserScript==

(function () {
  'use strict';

  // ---------------------------------------------------------------- config
  const CFG = {
    target: '2026-11-06T08:00:00+02:00',   // drop time (Bucharest)
    productPath: '/pd/DSSJM77BM',          // the 1-leu Beach Please 5-day pass
    landLateMs: 80,                        // aim for the reload to land this long after T
    retryWindowMs: 25000,                  // after T: auto-reload if no buy button, within this window
    firstRetryMs: 700,
    retryEveryMs: 1500,
    maxRetries: 8,
    rehearsalLeadMs: 20000,
  };

  const BUY_PHRASES = ['adauga in cos'];
  const NEXT_PHRASES = [
    'vezi detalii cos', 'vezi cosul', 'mergi in cos',
    'continua', 'plaseaza comanda', 'finalizeaza comanda', 'trimite comanda', 'plateste',
  ];
  const NEXT_EXCLUDE = ['cumparaturi', 'cumparaturile'];
  const SOLD_OUT_PHRASES = ['stoc epuizat', 'indisponibil', 'nu mai este disponibil'];
  const DONE_PHRASES = ['multumim pentru comanda', 'comanda ta a fost', 'comanda a fost plasata', 'comanda a fost inregistrata'];

  // ----------------------------------------------------------------- state
  const KEY = 'bpd.state.v1';
  const load = () => { try { return JSON.parse(sessionStorage.getItem(KEY)) || null; } catch (e) { return null; } };
  const save = (s) => { try { sessionStorage.setItem(KEY, JSON.stringify(s)); } catch (e) { /* ignore */ } };
  const clear = () => { try { sessionStorage.removeItem(KEY); } catch (e) { /* ignore */ } };

  const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
  const median = (a) => { const b = [...a].sort((x, y) => x - y); return b.length ? b[Math.floor(b.length / 2)] : 0; };
  const norm = (s) => (s || '').normalize('NFD').replace(/[̀-ͯ]/g, '').toLowerCase().replace(/\s+/g, ' ').trim();
  const bodyText = () => norm(document.body ? document.body.innerText : '');

  const path = location.pathname;
  const isProductPage = /\/pd\//.test(path);
  const productKey = (path.match(/\/pd\/[^/]+/) || [path])[0];
  const isTargetProduct = path.includes(CFG.productPath);
  const realTarget = Date.parse(CFG.target);

  let st = load();
  let offset = st && typeof st.offset === 'number' ? st.offset : 0;   // serverTime - localTime (ms)
  const serverNow = () => Date.now() + offset;

  // ------------------------------------------------------------ clock sync
  // Uses eMAG's own Date header. Each response pins the server's second; the
  // request/response window pins our local time. Intersecting many samples
  // narrows the offset to roughly half the round-trip time.
  async function syncClock(n = 14) {
    const samples = [];
    for (let i = 0; i < n; i++) {
      const t0 = Date.now();
      let res;
      try {
        res = await fetch(location.origin + '/?_bpd=' + t0 + '_' + i, { method: 'HEAD', cache: 'no-store', credentials: 'same-origin' });
      } catch (e) { await sleep(150); continue; }
      const t1 = Date.now();
      const d = res.headers.get('date');
      const age = res.headers.get('age');
      if (!d || (age && Number(age) > 0)) { await sleep(120); continue; }
      const S = Date.parse(d);
      if (!isNaN(S)) samples.push({ t0, t1, S });
      await sleep(90 + Math.random() * 110);
    }
    if (samples.length < 3) return null;
    const rtt = median(samples.map((s) => s.t1 - s.t0));
    let lo = -Infinity, hi = Infinity;
    for (const s of samples) { lo = Math.max(lo, s.S - s.t1); hi = Math.min(hi, s.S + 1000 - s.t0); }
    if (lo <= hi) return { offset: (lo + hi) / 2, err: (hi - lo) / 2, rtt, n: samples.length };
    const est = median(samples.map((s) => s.S + 500 - (s.t0 + s.t1) / 2));
    return { offset: est, err: 500 + rtt / 2, rtt, n: samples.length, coarse: true };
  }

  // ----------------------------------------------------------------- audio
  let actx = null;
  function initAudio() {
    try { actx = actx || new (window.AudioContext || window.webkitAudioContext)(); actx.resume(); } catch (e) { actx = null; }
  }
  function beep(freq = 880, dur = 0.08, vol = 0.18) {
    if (!actx) return;
    try {
      const o = actx.createOscillator(), g = actx.createGain();
      o.type = 'sine'; o.frequency.value = freq;
      g.gain.setValueAtTime(vol, actx.currentTime);
      g.gain.exponentialRampToValueAtTime(0.0001, actx.currentTime + dur);
      o.connect(g).connect(actx.destination); o.start(); o.stop(actx.currentTime + dur + 0.02);
    } catch (e) { /* ignore */ }
  }

  // ------------------------------------------------------------- overlay UI
  const css = `
  #bpd{position:fixed;top:14px;right:14px;z-index:2147483647;width:330px;font:13px/1.4 -apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,sans-serif;
    color:#f4f1ff;background:rgba(18,14,32,.86);backdrop-filter:blur(14px) saturate(160%);-webkit-backdrop-filter:blur(14px) saturate(160%);
    border:1px solid rgba(255,255,255,.12);border-radius:18px;box-shadow:0 18px 50px rgba(0,0,0,.45),inset 0 1px 0 rgba(255,255,255,.08);padding:16px 16px 14px;}
  #bpd *{box-sizing:border-box}
  #bpd .bpd-top{display:flex;justify-content:space-between;align-items:center;margin-bottom:6px}
  #bpd .bpd-brand{font-weight:700;letter-spacing:.14em;font-size:10.5px;text-transform:uppercase;background:linear-gradient(90deg,#ffb36b,#ff4f8b,#9b6bff);-webkit-background-clip:text;background-clip:text;color:transparent}
  #bpd .bpd-x{cursor:pointer;opacity:.5;font-size:16px;line-height:1;padding:2px 4px}
  #bpd .bpd-x:hover{opacity:1}
  #bpd .bpd-clock{font:600 40px/1.05 ui-monospace,"SF Mono",Menlo,Consolas,monospace;letter-spacing:-.02em;font-variant-numeric:tabular-nums;margin:4px 0 2px}
  #bpd .bpd-clock small{font-size:20px;opacity:.6}
  #bpd .bpd-sub{opacity:.62;font-size:11.5px;font-variant-numeric:tabular-nums}
  #bpd .bpd-status{margin-top:10px;padding:9px 11px;border-radius:11px;background:rgba(255,255,255,.06);font-weight:600}
  #bpd .bpd-status.go{background:linear-gradient(90deg,#ff4f8b,#ff8a3d);color:#fff;animation:bpdPulse 0.9s ease-in-out infinite}
  #bpd .bpd-status.ok{background:rgba(64,214,140,.18);color:#8ff0bf}
  #bpd .bpd-status.warn{background:rgba(255,196,0,.15);color:#ffd866}
  #bpd .bpd-row{display:flex;gap:8px;margin-top:10px}
  #bpd button.bpd-b{all:unset;cursor:pointer;flex:1;text-align:center;padding:9px 0;border-radius:11px;font-weight:700;font-size:12.5px;background:rgba(255,255,255,.09);color:#f4f1ff}
  #bpd button.bpd-b:hover{background:rgba(255,255,255,.16)}
  #bpd button.bpd-b.primary{background:linear-gradient(90deg,#ff4f8b,#9b6bff);color:#fff}
  #bpd button.bpd-b.armed{background:rgba(64,214,140,.22);color:#8ff0bf}
  #bpd .bpd-bar{height:3px;border-radius:3px;background:rgba(255,255,255,.08);margin-top:10px;overflow:hidden}
  #bpd .bpd-bar i{display:block;height:100%;width:0;background:linear-gradient(90deg,#ffb36b,#ff4f8b,#9b6bff);transition:width .1s linear}
  @keyframes bpdPulse{0%,100%{filter:brightness(1)}50%{filter:brightness(1.25)}}
  @keyframes bpdGlow{0%,100%{box-shadow:0 0 0 4px #ff4f8b,0 0 22px 6px rgba(255,79,139,.75)}50%{box-shadow:0 0 0 6px #ffb36b,0 0 38px 14px rgba(255,138,61,.85)}}
  .bpd-glow{animation:bpdGlow .8s ease-in-out infinite !important;outline:none !important;position:relative;z-index:2147483646 !important;border-radius:8px}
  `;
  const style = document.createElement('style'); style.textContent = css; document.documentElement.appendChild(style);

  const ui = document.createElement('div');
  ui.id = 'bpd';
  ui.innerHTML = `
    <div class="bpd-top"><span class="bpd-brand">Beach Please · Drop Assist</span><span class="bpd-x" title="Hide">×</span></div>
    <div class="bpd-clock">--:--<small>.---</small></div>
    <div class="bpd-sub bpd-l1">syncing with eMAG clock…</div>
    <div class="bpd-sub bpd-l2">&nbsp;</div>
    <div class="bpd-bar"><i></i></div>
    <div class="bpd-status">Loading…</div>
    <div class="bpd-row"></div>`;
  const $ = (sel) => ui.querySelector(sel);
  $('.bpd-x').onclick = () => { ui.style.display = 'none'; };
  const setStatus = (text, kind = '') => { const s = $('.bpd-status'); s.className = 'bpd-status ' + kind; s.textContent = text; };
  const setButtons = (defs) => {
    const row = $('.bpd-row'); row.innerHTML = '';
    for (const d of defs) {
      const b = document.createElement('button'); b.className = 'bpd-b ' + (d.cls || ''); b.textContent = d.label; b.onclick = d.onClick; row.appendChild(b);
    }
    row.style.display = defs.length ? 'flex' : 'none';
  };
  const fmtCountdown = (ms) => {
    const neg = ms < 0; ms = Math.abs(ms);
    const h = Math.floor(ms / 3600000), m = Math.floor(ms / 60000) % 60, s = Math.floor(ms / 1000) % 60, f = Math.floor(ms % 1000);
    const main = (h ? String(h).padStart(2, '0') + ':' : '') + String(m).padStart(2, '0') + ':' + String(s).padStart(2, '0');
    return { main: (neg ? 'T+' : '') + main, frac: '.' + String(f).padStart(3, '0') };
  };
  const fmtClock = (ms) => new Date(ms).toLocaleTimeString('ro-RO', { timeZone: 'Europe/Bucharest', hour12: false }) + '.' + String(Math.floor(((ms % 1000) + 1000) % 1000)).padStart(3, '0');
  const setClock = (ms) => { const f = fmtCountdown(ms); $('.bpd-clock').innerHTML = `${f.main}<small>${f.frac}</small>`; };

  // --------------------------------------------------------- page helpers
  function visible(el) {
    if (!el || el.disabled) return false;
    const r = el.getBoundingClientRect();
    if (r.width < 2 || r.height < 2) return false;
    const cs = getComputedStyle(el);
    return cs.visibility !== 'hidden' && cs.display !== 'none' && Number(cs.opacity) > 0.05;
  }
  function labelOf(el) { return norm(el.innerText || el.value || el.getAttribute('aria-label') || el.title || ''); }
  function candidates() {
    return Array.from(document.querySelectorAll('button, a, input[type=submit], input[type=button], [role=button]')).filter((el) => !ui.contains(el));
  }
  function findBuyButton() {
    const direct = document.querySelector('form.main-product-form button[type=submit], .main-product-form .yeahIWantThisProduct, button.yeahIWantThisProduct');
    if (direct && visible(direct)) return direct;
    for (const el of candidates()) {
      const t = labelOf(el);
      if (t.length <= 40 && BUY_PHRASES.some((p) => t.includes(p)) && visible(el)) return el;
    }
    return null;
  }
  function findNextButton() {
    const els = candidates();
    for (const p of NEXT_PHRASES) {
      for (const el of els) {
        const t = labelOf(el);
        if (t.length > 40 || !t.includes(p) || NEXT_EXCLUDE.some((x) => t.includes(x))) continue;
        if (visible(el)) return el;
      }
    }
    return null;
  }
  let glowing = null;
  function glow(el, focus) {
    if (glowing === el) return false;
    if (glowing) glowing.classList.remove('bpd-glow');
    glowing = el;
    if (!el) return false;
    el.classList.add('bpd-glow');
    el.scrollIntoView({ block: 'center', behavior: 'instant' in document.documentElement.style ? 'instant' : 'auto' });
    const ae = document.activeElement;
    const typing = ae && (ae.tagName === 'INPUT' || ae.tagName === 'TEXTAREA' || ae.tagName === 'SELECT' || ae.isContentEditable);
    if (focus && !typing) { try { el.focus({ preventScroll: true }); } catch (e) { /* ignore */ } }
    return true;
  }
  function priceText() {
    const p = document.querySelector('.product-new-price');
    if (!p) return '';
    const c = p.cloneNode(true);
    c.querySelectorAll('sup').forEach((s) => { s.textContent = ',' + s.textContent; });
    return c.textContent.replace(/\s+/g, ' ').trim();
  }
  function looksLikeHumanCheck() {
    if (document.querySelector('#captcha-container, .amzn-captcha-modal, iframe[src*="captcha"], script[src*="captcha"]')) return true;
    return !document.querySelector('h1') && bodyText().length < 600;
  }

  // ------------------------------------------------------------ modes
  let tickTimer = null;
  const hiddenWarn = () => (document.visibilityState === 'hidden' ? ' ⚠ keep this tab in front' : '');

  // Countdown before T on the drop product (or rehearsal product).
  async function preMode(target, rehearsal) {
    document.body.appendChild(ui);
    setClock(target - Date.now());
    setStatus(rehearsal ? 'Rehearsal: syncing…' : 'Syncing with eMAG clock…');
    setButtons([]);

    const doSync = async () => {
      const r = await syncClock();
      if (r) {
        offset = r.offset;
        st = Object.assign(st || {}, { offset: r.offset, err: r.err, rtt: r.rtt, syncedAt: Date.now() });
        save(st);
      }
      return r;
    };
    let sync = (st && st.syncedAt && Date.now() - st.syncedAt < 5 * 60000 && st.target === target) ? { offset: st.offset, err: st.err, rtt: st.rtt } : await doSync();
    if (!sync) sync = { offset: 0, err: 1000, rtt: 150, coarse: true };

    const fireServer = () => target - Math.min((sync.rtt || 0) / 2, 400) + CFG.landLateMs + Math.min(sync.err || 0, 300);
    const describe = () => {
      $('.bpd-l1').textContent = `eMAG clock ${offset >= 0 ? '+' : ''}${Math.round(offset)} ms vs yours (±${Math.round(sync.err)} ms)${sync.coarse ? ' · coarse' : ''}`;
      $('.bpd-l2').textContent = `latency ${Math.round(sync.rtt)} ms → reload fires at ${fmtClock(fireServer())}`;
    };
    describe();

    st = Object.assign(st || {}, { target, product: productKey, rehearsal: !!rehearsal, fired: false, retries: 0 });
    save(st);

    const arm = () => {
      initAudio(); beep(660, 0.1);
      st.armed = true; save(st); render();
      schedule();
    };
    const disarm = () => { st.armed = false; save(st); render(); };
    const render = () => {
      const btns = [];
      if (st.armed) btns.push({ label: '✓ ARMED', cls: 'armed', onClick: disarm });
      else btns.push({ label: 'ARM', cls: 'primary', onClick: arm });
      btns.push({ label: 'Resync', onClick: async () => { setStatus('Resyncing…'); const r = await doSync(); if (r) sync = r; describe(); render(); } });
      if (!rehearsal) btns.push({ label: 'Rehearse 20s', onClick: () => startRehearsal() });
      else btns.push({ label: 'Cancel', onClick: () => { clear(); location.reload(); } });
      setButtons(btns);
      const buy = findBuyButton();
      const price = priceText();
      if (!st.armed) setStatus((rehearsal ? 'Rehearsal ready. ' : '') + 'Press ARM — it reloads this page for you at the exact moment.', 'warn');
      else setStatus(`Armed. Hands on the trackpad.${buy ? ' Buy button is already on the page' + (price ? ' (' + price + ')' : '') + '.' : ''}`, 'ok');
    };
    render();

    let lastBeepSec = null, resynced = false, fireTimer = null;
    const schedule = () => {
      clearTimeout(fireTimer);
      const step = () => {
        if (!st.armed) return;
        const rem = fireServer() - serverNow();
        if (rem <= 0) return fire();
        if (rem > 30) { fireTimer = setTimeout(step, Math.min(rem - 25, 250)); return; }
        const end = Date.now() + rem; while (Date.now() < end) { /* spin the last few ms */ }
        fire();
      };
      step();
    };
    const fire = () => {
      st.fired = true; st.firedAt = serverNow(); st.retries = 0; save(st);
      beep(1320, 0.35, 0.25);
      setStatus('RELOADING NOW', 'go');
      location.reload();
    };
    if (st.armed) { initAudio(); schedule(); }

    clearInterval(tickTimer);
    tickTimer = setInterval(async () => {
      const rem = target - serverNow();
      setClock(rem);
      const total = Math.max(1, target - (st.syncedAt || Date.now() - 60000));
      $('.bpd-bar i').style.width = Math.max(0, Math.min(100, 100 - (rem / total) * 100)) + '%';
      const sec = Math.ceil(rem / 1000);
      if (st.armed && sec <= 10 && sec >= 1 && sec !== lastBeepSec) { lastBeepSec = sec; beep(sec <= 3 ? 990 : 880, 0.07); }
      if (!resynced && rem < 45000 && rem > 20000 && st.syncedAt && Date.now() - st.syncedAt > 60000) {
        resynced = true; const r = await doSync(); if (r) { sync = r; describe(); if (st.armed) schedule(); }
      }
      if (document.visibilityState === 'hidden' && st.armed) setStatus('Armed —' + hiddenWarn(), 'warn');
    }, 47);
  }

  function startRehearsal() {
    const target = serverNow() + CFG.rehearsalLeadMs;
    st = { target, product: productKey, rehearsal: true, armed: false, fired: false, retries: 0, offset, err: st && st.err, rtt: st && st.rtt, syncedAt: st && st.syncedAt };
    save(st);
    clearInterval(tickTimer);
    preMode(target, true);
  }

  // After T on the product page: find the buy button, glow it, or retry the reload.
  async function postMode() {
    document.body.appendChild(ui);
    const target = st ? st.target : realTarget;
    const startClock = () => {
      clearInterval(tickTimer);
      tickTimer = setInterval(() => setClock(target - serverNow()), 47);
    };
    startClock();
    $('.bpd-l1').textContent = st && st.rehearsal ? 'Rehearsal run' : 'Drop is live';
    $('.bpd-l2').textContent = st && st.fired ? `reload landed at ${fmtClock(st.firedAt)}` : '';
    $('.bpd-bar').style.display = 'none';
    setButtons([{ label: 'Reset', onClick: () => { clear(); location.reload(); } }]);

    // Give the page up to 1.5 s to render the buy box.
    const t0 = Date.now();
    let buy = findBuyButton();
    while (!buy && Date.now() - t0 < 1500) { await sleep(60); buy = findBuyButton(); }

    if (buy) {
      initAudio(); beep(1320, 0.25, 0.25);
      glow(buy, true);
      const price = priceText();
      setStatus(`GO — press Enter or click the glowing button${price ? ' · ' + price : ''}`, 'go');
      guideMode(true);
      return;
    }

    if (looksLikeHumanCheck()) { setStatus('eMAG is showing a human check. Solve it, then press F5.', 'warn'); return; }

    const sold = SOLD_OUT_PHRASES.some((p) => bodyText().includes(p));
    const inWindow = st && st.armed && serverNow() - target < CFG.retryWindowMs;
    if (inWindow && st.retries < CFG.maxRetries) {
      const wait = st.retries === 0 ? CFG.firstRetryMs : CFG.retryEveryMs;
      st.retries += 1; save(st);
      setStatus(`${sold ? 'Shows unavailable' : 'No buy button yet'} — reloading in ${(wait / 1000).toFixed(1)}s (${st.retries}/${CFG.maxRetries})`, 'warn');
      setTimeout(() => location.reload(), wait);
      return;
    }
    setStatus(sold ? 'Shows sold out / unavailable. Press F5 to check again.' : 'No buy button found. Press F5 to try again.', 'warn');
  }

  // Through cart and checkout: keep the next button glowing, run the stopwatch.
  function guideMode(onProductPage) {
    if (!ui.isConnected) document.body.appendChild(ui);
    const target = st.target;
    clearInterval(tickTimer);
    tickTimer = setInterval(() => setClock(target - serverNow()), 47);
    $('.bpd-bar').style.display = 'none';
    $('.bpd-l1').textContent = (st.rehearsal ? 'Rehearsal · ' : '') + 'stopwatch since drop time';
    setButtons([{ label: 'Reset', onClick: () => { clear(); if (glowing) glowing.classList.remove('bpd-glow'); ui.remove(); clearInterval(tickTimer); } }]);

    const check = () => {
      const txt = bodyText();
      if (DONE_PHRASES.some((p) => txt.includes(p)) || /thank|order-completed|comanda-finalizata/i.test(location.pathname)) {
        clearInterval(tickTimer);
        const took = serverNow() - target;
        setClock(target - serverNow());
        setStatus(`Order placed at T+${(took / 1000).toFixed(1)}s. Check your email.`, 'ok');
        glow(null);
        st.done = true; save(st);
        return;
      }
      let el = onProductPage ? findBuyButton() : null;
      const next = findNextButton();
      if (next) el = next;
      if (el && glow(el, true)) {
        const label = (el.innerText || el.value || '').replace(/\s+/g, ' ').trim();
        setStatus(`Next: “${label}” — Enter or click`, 'go');
        beep(1100, 0.06);
      } else if (!el && !glowing) {
        setStatus('Fill anything required, then the next button will glow.', '');
      }
    };
    check();
    let pending = false;
    new MutationObserver(() => {
      if (pending) return; pending = true;
      setTimeout(() => { pending = false; if (glowing && !glowing.isConnected) glowing = null; check(); }, 120);
    }).observe(document.body, { childList: true, subtree: true, attributes: true, attributeFilter: ['class', 'style', 'disabled'] });
  }

  // ------------------------------------------------------------- routing
  function route() {
    const now = serverNow();
    if (st && st.done) { if (now - st.target > 30 * 60000) clear(); return; }

    if (isProductPage && st && st.fired && st.product === productKey && now - st.target < 5 * 60000) return postMode();

    if (isTargetProduct) {
      if (st && st.rehearsal) { clear(); st = null; }
      if (now < realTarget - 5) return preMode(realTarget, false);
      st = Object.assign(st || {}, { target: realTarget, product: productKey });
      save(st);
      return postMode();
    }

    if (isProductPage && st && st.rehearsal && !st.fired && st.product === productKey && now < st.target) return preMode(st.target, true);

    if (st && st.fired && now - st.target < 15 * 60000) return guideMode(isProductPage);

    if (isProductPage) {
      // Any other product page: offer a rehearsal so you can time your checkout.
      document.body.appendChild(ui);
      $('.bpd-clock').innerHTML = 'Rehearsal';
      $('.bpd-l1').textContent = 'Not the drop product.';
      $('.bpd-l2').textContent = 'Practice the full flow here (stop before paying).';
      $('.bpd-bar').style.display = 'none';
      setStatus('Fake “08:00” 20 s from now on this page.', '');
      setButtons([{ label: 'Rehearse 20s', cls: 'primary', onClick: () => { initAudio(); startRehearsal(); } }]);
    }
  }

  route();
})();
