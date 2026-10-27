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
  script.addEventListener('error', () => { if (status) status.textContent = 'The external GoatCounter script could not load.'; });
  script.addEventListener('load', () => {
    if (status) status.textContent = 'GoatCounter script loaded. The test button is ready.';
    const notice = document.getElementById('analytics-notice');
    if (notice) notice.hidden = false;
  });
  document.head.appendChild(script);
})();
