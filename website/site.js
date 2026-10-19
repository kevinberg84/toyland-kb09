(() => {
  'use strict';
  const names = ['Minty', 'Seen Some Shit', 'Vintage, Baby!', 'Sold As-Is', 'Fire Hazard'];
  const frames = [...document.querySelectorAll('.panel-frame')];
  const buttons = [...document.querySelectorAll('[data-select-age]')];
  const slider = document.getElementById('age-scrubber');
  const play = document.getElementById('play-pause');
  const speed = document.getElementById('speed');
  const reduced = window.matchMedia('(prefers-reduced-motion: reduce)');
  let age = 0;
  let timer = null;
  function selectAge(value) {
    age = Math.max(0, Math.min(4, Number(value)));
    frames.forEach((frame, index) => {
      frame.classList.toggle('active', index === age);
      frame.setAttribute('aria-hidden', String(index !== age));
    });
    buttons.forEach((button, index) => {
      button.setAttribute('aria-checked', String(index === age));
      button.tabIndex = index === age ? 0 : -1;
    });
    slider.value = age;
    slider.setAttribute('aria-valuetext', names[age]);
    document.getElementById('age-name').textContent = names[age];
  }
  function pause() {
    if (timer !== null) window.clearInterval(timer);
    timer = null;
    play.textContent = 'Play cycle';
    play.setAttribute('aria-pressed', 'false');
  }
  function start() {
    pause();
    timer = window.setInterval(() => selectAge((age + 1) % 5), 2800 / Number(speed.value));
    play.textContent = 'Pause cycle';
    play.setAttribute('aria-pressed', 'true');
  }
  buttons.forEach((button, index) => {
    button.addEventListener('click', () => { pause(); selectAge(index); });
    button.addEventListener('keydown', event => {
      if (!['ArrowLeft', 'ArrowRight', 'ArrowUp', 'ArrowDown', 'Home', 'End'].includes(event.key)) return;
      event.preventDefault();
      const next = event.key === 'Home' ? 0 : event.key === 'End' ? 4 : (index + (['ArrowRight', 'ArrowDown'].includes(event.key) ? 1 : 4)) % 5;
      pause(); selectAge(next); buttons[next].focus();
    });
  });
  slider.addEventListener('input', () => { pause(); selectAge(slider.value); });
  play.addEventListener('click', () => timer === null ? start() : pause());
  speed.addEventListener('change', () => { if (timer !== null) start(); });
  document.addEventListener('visibilitychange', () => { if (document.hidden) pause(); });
  reduced.addEventListener('change', () => { if (reduced.matches) pause(); });
  selectAge(0);
  // Leave the visitor in control; reduced-motion users never start automatically.
  if (!reduced.matches) start();

  const dialog = document.getElementById('image-dialog');
  const dialogImage = document.getElementById('dialog-image');
  function enlarge(src, alt) {
    pause();
    dialogImage.src = src;
    dialogImage.alt = alt;
    document.getElementById('image-title').textContent = alt;
    dialog.showModal();
  }
  document.getElementById('enlarge-panel').addEventListener('click', () => enlarge(frames[age].src, `KB-09 · ${names[age]}`));
  document.querySelectorAll('[data-enlarge]').forEach(button => button.addEventListener('click', () => {
    const image = button.closest('figure').querySelector('img');
    enlarge(button.dataset.enlarge, image.alt);
  }));
  document.getElementById('close-dialog').addEventListener('click', () => dialog.close());
  dialog.addEventListener('click', event => { if (event.target === dialog) { const r = dialog.getBoundingClientRect(); if (event.clientX < r.left || event.clientX > r.right || event.clientY < r.top || event.clientY > r.bottom) dialog.close(); } });

  // Every link works without the API. Hydration only adds convenient direct ZIPs.
  const repo = 'https://github.com/kevinberg84/toyland-kb09';
  const controller = new AbortController();
  const timeout = window.setTimeout(() => controller.abort(), 6000);
  fetch('https://api.github.com/repos/kevinberg84/toyland-kb09/releases/latest', { signal: controller.signal })
    .then(response => { if (!response.ok) throw new Error('Use the release-page fallback'); return response.json(); })
    .then(release => {
      if (!Array.isArray(release.assets) || release.draft || release.prerelease) return;
      document.querySelectorAll('[data-platform]').forEach(link => {
        const platform = link.dataset.platform;
        const asset = release.assets.find(item => item.name.toLowerCase().includes(platform) && item.name.toLowerCase().endsWith('.zip'));
        if (asset && asset.browser_download_url.startsWith(repo + '/releases/download/')) link.href = asset.browser_download_url;
      });
      if (/^v?\d+\.\d+\.\d+$/.test(release.tag_name)) document.getElementById('release-label').textContent = `Latest release · ${release.tag_name}`;
    })
    .catch(() => { /* Permanent /releases/latest links remain available. */ })
    .finally(() => window.clearTimeout(timeout));
})();
