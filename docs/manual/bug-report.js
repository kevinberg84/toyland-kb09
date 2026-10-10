(() => {
  'use strict';
  const form = document.getElementById('bug-report-form');
  if (!form) return;
  const status = document.getElementById('report-status');
  const send = document.getElementById('report-send');
  const saved = document.getElementById('report-copy');
  const started = form.elements.namedItem('startedAt');
  started.value = String(Date.now());
  const endpoint = form.getAttribute('action') || '';
  const configured = /^https:\/\/script\.google\.com\/macros\/s\/[A-Za-z0-9_-]+\/exec$/.test(endpoint);
  send.disabled = !configured;
  status.textContent = configured ? 'Internet is needed only when you submit. Confirmation opens in a new tab; your answers stay here.' :
    'Online reporting is not connected yet. You can complete and save a report locally, then share it with Kevin.';
  form.addEventListener('submit', event => {
    if (!configured) { event.preventDefault(); return; }
    if (!form.reportValidity()) { event.preventDefault(); return; }
    if (Date.now() - Number(started.value) < 3000) {
      event.preventDefault(); status.textContent = 'Please take a moment to check your answers, then submit again.'; return;
    }
    status.textContent = 'Check the new tab for confirmation. If it does not open or reports an error, your answers remain here: use Save report locally.';
    // Standard browser POST; no fetch, CORS workaround, or form reset.
  });
  document.getElementById('report-save').addEventListener('click', () => {
    const value = name => form.elements.namedItem(name).value.trim();
    const text = ['KB-09 bug report', 'Version: '+value('version'), 'OS: '+value('os'),
      'DAW / host: '+value('host'), 'Category: '+value('category'),
      'Contact (optional): '+(value('email') || 'Not provided'),
      '\nSteps to reproduce:\n'+value('steps'), '\nExpected behavior:\n'+value('expected'),
      '\nActual behavior:\n'+value('actual')].join('\n');
    saved.value = text;
    document.getElementById('report-copy-details').open = true;
    const url = URL.createObjectURL(new Blob([text], {type:'text/plain;charset=utf-8'}));
    const link = document.createElement('a'); link.href = url;
    link.download = 'KB-09-bug-report-'+new Date().toISOString().slice(0,10)+'.txt';
    document.body.append(link); link.click(); link.remove();
    setTimeout(() => URL.revokeObjectURL(url), 30000);
    status.textContent = 'A text-file download was requested. If it was blocked, copy the report from the box below. Your form answers remain unchanged.';
  });
})();
