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
  let screenshot = null, readingScreenshot = false, screenshotError = false, readGeneration = 0;
  const shotInput = document.getElementById('report-screenshot');
  const shotField = key => form.elements.namedItem(key);
  const resetShot = () => { screenshot=null; readingScreenshot=false; screenshotError=false;
    ['shotName','shotType','shotData'].forEach(key=>shotField(key).value=''); send.disabled=!configured; };
  shotInput.addEventListener('change', () => {
    const generation = ++readGeneration; resetShot();
    const file = shotInput.files && shotInput.files[0];
    if (!file) return;
    if (!['image/png','image/jpeg'].includes(file.type) || file.size > 2097152 || !file.size || file.name.length > 200) {
      screenshotError=true; send.disabled=true;
      status.textContent='Choose a PNG or JPEG screenshot up to 2 MB, or remove the attachment to submit without one.'; return;
    }
    readingScreenshot=true; send.disabled=true; status.textContent='Preparing your screenshot locally. Nothing has been sent.';
    const reader = new FileReader();
    reader.onload = () => {
      if (generation !== readGeneration) return;
      screenshot={name:file.name,type:file.type,data:String(reader.result).split(',')[1]};
      shotField('shotName').value=screenshot.name; shotField('shotType').value=screenshot.type; shotField('shotData').value=screenshot.data;
      readingScreenshot=false; send.disabled=!configured;
      status.textContent='Screenshot attached. Check it for private information before submitting. '+(configured?'Confirmation opens in a new tab.':'Online reporting is not connected yet; you can save the report locally.');
    };
    reader.onerror = () => {if(generation!==readGeneration)return;readingScreenshot=false;screenshotError=true;status.textContent='The screenshot could not be read. Choose it again or remove the attachment.';};
    reader.readAsDataURL(file);
  });
  document.getElementById('report-remove-screenshot').addEventListener('click',()=>{++readGeneration;shotInput.value='';resetShot();status.textContent='Screenshot removed. Your other answers remain unchanged.';});
  send.disabled = !configured;
  status.textContent = configured ? 'Internet is needed only when you submit. Confirmation opens in a new tab; your answers stay here.' :
    'Online reporting is not connected yet. You can complete and save a report locally, then share it with Kevin.';
  form.addEventListener('submit', event => {
    if (!configured) { event.preventDefault(); return; }
    if (readingScreenshot || screenshotError) {event.preventDefault();status.textContent='Finish preparing, replace, or remove the screenshot before submitting.';return;}
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
      '\nActual behavior:\n'+value('actual'), '\nScreenshot: '+(screenshot?screenshot.name:'Not attached')].join('\n');
    saved.value = text;
    document.getElementById('report-copy-details').open = true;
    const escape = text => text.replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
    const contents = screenshot ? '<!doctype html><meta charset="utf-8"><title>KB-09 bug report</title><pre style="white-space:pre-wrap">'+escape(text)+'</pre><img alt="Attached screenshot" style="max-width:100%" src="data:'+screenshot.type+';base64,'+screenshot.data+'">' : text;
    const url = URL.createObjectURL(new Blob([contents], {type:screenshot?'text/html;charset=utf-8':'text/plain;charset=utf-8'}));
    const link = document.createElement('a'); link.href = url;
    link.download = 'KB-09-bug-report-'+new Date().toISOString().slice(0,10)+(screenshot?'.html':'.txt');
    document.body.append(link); link.click(); link.remove();
    setTimeout(() => URL.revokeObjectURL(url), 30000);
    status.textContent = (screenshot?'An HTML report containing your screenshot was requested. ':'A text-file download was requested. ')+
      'If the download was blocked, copy the answers below and keep the original screenshot separately. Your form answers remain unchanged.';
  });
})();
