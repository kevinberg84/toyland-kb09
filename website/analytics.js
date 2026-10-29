/* Website-only analytics. Local previews are ignored except explicit QA. */
(() => {
  'use strict';
  const production = location.protocol === 'https:' && location.hostname === 'kevinberg84.github.io';
  const qa = ['127.0.0.1', 'localhost'].includes(location.hostname)
    && location.pathname === '/analytics-qa/'
    && document.querySelector('meta[name="analytics-qa"]')?.content === 'true';
  if (!production && !qa) return;
  const status = qa ? document.getElementById('qa-status') : null;
  if (status) status.textContent = 'Analytics loader started.';
  if (navigator.globalPrivacyControl || navigator.doNotTrack === '1') {
    if (status) status.textContent = 'Analytics skipped because this browser requests privacy protection.';
    return;
  }
  const endpoint = document.querySelector('meta[name="goatcounter-endpoint"]')?.content || '';
  if (!/^https:\/\/[a-z0-9][a-z0-9-]*\.goatcounter\.com\/count$/.test(endpoint)) return;
  const script = document.createElement('script');
  script.async = true;
  script.src = 'https://gc.zgo.at/count.js';
  script.dataset.goatcounter = endpoint;
  if (qa) script.dataset.goatcounterSettings = JSON.stringify({
    allow_local: true,
    path: 'qa-prelaunch-visit',
    title: 'TEST ONLY: KB-09 prelaunch analytics visit',
    referrer: 'Prelaunch QA'
  });
  const pending = new Map();
  function flushDemoEvents() {
    const counter = window.goatcounter;
    if (typeof counter?.count !== 'function' || counter.filter?.()) return;
    for (const [path, title] of pending) {
      counter.count({ path: qa ? `qa-${path}` : path, title, event: true });
      pending.delete(path);
    }
  }
  const demo = document.querySelector('.demo-player audio');
  if (demo) {
    const recorded = new Set();
    let listened = 0;
    let sample = null;
    function record(path, title) {
      if (recorded.has(path)) return;
      recorded.add(path);
      pending.set(path, title);
      flushDemoEvents();
    }
    function measure() {
      const now = performance.now();
      if (sample && !demo.seeking) {
        // Bound media progress by elapsed time: seeking ahead is not listening.
        const progress = Math.max(0, demo.currentTime - sample.position);
        listened += Math.min(progress, Math.max(0, (now - sample.time) / 1000));
        if (listened >= 30) record('demo-listened-30s', 'Demo: listened for 30 seconds');
      }
      sample = !demo.paused && !demo.seeking ? { position: demo.currentTime, time: now } : null;
    }
    demo.addEventListener('playing', () => {
      record('demo-started', 'Demo: started playing');
      sample = { position: demo.currentTime, time: performance.now() };
    });
    demo.addEventListener('timeupdate', measure);
    demo.addEventListener('pause', measure);
    demo.addEventListener('seeking', () => { sample = null; });
    demo.addEventListener('seeked', () => { sample = null; measure(); });
    demo.addEventListener('waiting', () => { measure(); sample = null; });
    demo.addEventListener('ended', () => {
      measure();
      record('demo-ended', 'Demo: reached the end');
    });
  }
  script.addEventListener('error', () => { if (status) status.textContent = 'The external GoatCounter script could not load.'; });
  script.addEventListener('load', () => {
    flushDemoEvents();
    if (status) status.textContent = 'GoatCounter script loaded. The test button is ready.';
    const notice = document.getElementById('analytics-notice');
    if (notice) notice.hidden = false;
  });
  document.head.appendChild(script);
})();
