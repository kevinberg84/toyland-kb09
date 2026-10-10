/** KB-09 Bug Reporting. Server-only: never bundle this file with the manual. */
function configureBugReporting() {
  PropertiesService.getScriptProperties().setProperty('REPORT_TO', 'kevinberg84@gmail.com');
  // Run once in the editor to authorize MailApp. This sends no email.
  return MailApp.getRemainingDailyQuota();
}

const REPORT_FIELDS = {
  version: 40, os: 120, host: 160, category: 40,
  steps: 3000, expected: 1500, actual: 3000, email: 254,
  website: 80, startedAt: 16, schema: 8, consent: 3,
  shotName: 200, shotType: 20, shotData: 2796204
};
const REPORT_CATEGORIES = ['Sound / MIDI', 'Interface / display', 'Installation / updates',
  'Crash / performance', 'Manual / documentation', 'Other'];

function reportError_(message) {
  const error = new Error(message); error.publicMessage = message; throw error;
}
function escapeReportHtml_(value) {
  return String(value).replace(/[&<>"']/g, c => ({'&':'&amp;', '<':'&lt;', '>':'&gt;', '"':'&quot;', "'":'&#39;'}[c]));
}
function reportPage_(success, message) {
  return HtmlService.createHtmlOutput('<!doctype html><html lang="en"><head><meta charset="utf-8">' +
    '<meta name="viewport" content="width=device-width,initial-scale=1"><title>KB-09 Bug Report</title>' +
    '<style>body{background:#f4f0e7;color:#252620;font:18px/1.65 system-ui;max-width:680px;margin:8vh auto;padding:24px}' +
    'main{border-top:5px solid #ac8540;padding-top:16px}h1{line-height:1.2}a{color:#695019}</style></head><body><main>' +
    '<p>TOYLAND KB-09</p><h1>' + (success ? 'Report submitted. Thank you!' : 'Report not confirmed') + '</h1><p>' +
    escapeReportHtml_(message) + '</p><p>' + (success ? 'You can close this tab and return to your manual.' :
      'Return to the manual tab. Your answers remain there. Choose Save report locally to keep a copy, or try again later. If the network failed after sending, the report may already have arrived.') +
    '</p></main></body></html>').setTitle('KB-09 Bug Report');
}
function doGet() {
  return reportPage_(false, 'Open the Bug Report questionnaire in your KB-09 User Manual to submit a report.');
}
function validateReport_(e) {
  if (!e || !e.postData || e.postData.length > 4500000 || e.queryString ||
      String(e.postData.type).split(';')[0] !== 'application/x-www-form-urlencoded')
    reportError_('This submission format is not supported. Please use the manual’s questionnaire.');
  const params = e.parameters || {}, report = {};
  if (Object.keys(params).some(key => !Object.prototype.hasOwnProperty.call(REPORT_FIELDS, key)))
    reportError_('The questionnaire contains an unsupported field.');
  Object.keys(REPORT_FIELDS).forEach(key => {
    const values = params[key] || [''];
    if (!Array.isArray(values) || values.length !== 1 || typeof values[0] !== 'string')
      reportError_('Please submit each questionnaire field only once.');
    const raw = values[0];
    if (raw.length > REPORT_FIELDS[key] || /[\x00-\x08\x0b\x0c\x0e-\x1f\x7f]/.test(raw))
      reportError_('One answer is too long or contains unsupported characters. Please shorten or edit it.');
    report[key] = key === 'shotData' ? raw : raw.replace(/\r\n?/g, '\n').trim();
  });
  ['version','os','host','category','steps','expected','actual'].forEach(key => {
    if (!report[key]) reportError_('Please complete every required answer.');
  });
  if (report.schema !== '1' || report.consent !== 'yes' || report.website ||
      !REPORT_CATEGORIES.includes(report.category)) reportError_('The submission could not be accepted. Please use the manual’s questionnaire.');
  const elapsed = Date.now() - Number(report.startedAt);
  if (!/^\d{13}$/.test(report.startedAt) || !Number.isFinite(elapsed) || elapsed < 3000 || elapsed > 30*86400000)
    reportError_('Please reopen the manual and take a moment to complete the questionnaire before submitting.');
  if (report.email && !/^[^\s@<>]+@[^\s@<>]+\.[^\s@<>]+$/.test(report.email))
    reportError_('Please check the contact email, or leave it blank.');
  return report;
}
function reportText_(r) {
  return ['KB-09 bug report', 'Version: '+r.version, 'OS: '+r.os, 'DAW / host: '+r.host,
    'Category: '+r.category, 'Contact (optional): '+(r.email || 'Not provided'),
    '\nSteps to reproduce:\n'+r.steps, '\nExpected behavior:\n'+r.expected,
    '\nActual behavior:\n'+r.actual, '\nScreenshot: '+(r.shotName || 'Not attached')].join('\n');
}
function screenshotAttachment_(r) {
  if (!r.shotData && !r.shotName && !r.shotType) return null;
  if (!r.shotName || !['image/png','image/jpeg'].includes(r.shotType) ||
      !r.shotData || r.shotData.length % 4 || !/^[A-Za-z0-9+/]*={0,2}$/.test(r.shotData))
    reportError_('Please choose a PNG or JPEG screenshot, no larger than 2 MB.');
  const bytes = Utilities.base64Decode(r.shotData);
  if (!bytes.length || bytes.length > 2097152 || Utilities.base64Encode(bytes) !== r.shotData)
    reportError_('The screenshot could not be accepted. Choose a PNG or JPEG up to 2 MB.');
  const signature = r.shotType === 'image/png' ? [137,80,78,71,13,10,26,10] : [255,216,255];
  if (bytes.length < signature.length || signature.some((b,i)=>(bytes[i]&255)!==b))
    reportError_('The screenshot format does not match its file type. Please use PNG or JPEG.');
  return Utilities.newBlob(bytes, r.shotType, 'KB-09-screenshot.'+(r.shotType==='image/png'?'png':'jpg'));
}
function doPost(e) {
  try {
    const report = validateReport_(e), body = reportText_(report), attachment = screenshotAttachment_(report);
    const fingerprint = Utilities.computeDigest(Utilities.DigestAlgorithm.SHA_256, body+report.shotData, Utilities.Charset.UTF_8)
      .map(b => ('0'+((b+256)%256).toString(16)).slice(-2)).join('');
    const lock = LockService.getScriptLock();
    if (!lock.tryLock(5000)) reportError_('Reporting is busy. Please try again in a few minutes.');
    try {
      const properties = PropertiesService.getScriptProperties(), cache = CacheService.getScriptCache();
      const recipient = properties.getProperty('REPORT_TO');
      // Fixed server configuration only. No form field ever chooses a recipient.
      if (!recipient || recipient !== 'kevinberg84@gmail.com') reportError_('Reporting is not configured yet. Please save your report locally.');
      if (cache.get(fingerprint)) return reportPage_(true, 'An identical report was already submitted recently; no duplicate email was sent.');
      const now = Date.now();
      let state;
      try { state = JSON.parse(properties.getProperty('RATE_STATE') || '{}'); } catch (_) { state = {}; }
      const day = new Date(now).toISOString().slice(0,10);
      if (state.day !== day) { state.day = day; state.daily = 0; }
      if (!state.start || now - state.start >= 600000) { state.start = now; state.recent = 0; }
      if (state.daily >= 30 || state.recent >= 5 || MailApp.getRemainingDailyQuota() < 1)
        reportError_('The reporting limit has been reached. Save your report locally and try again later.');
      // Reserve before sending, including failed attempts, to cap abuse safely.
      state.daily = (state.daily || 0)+1; state.recent = (state.recent || 0)+1;
      properties.setProperty('RATE_STATE', JSON.stringify(state));
      const reference = Utilities.getUuid();
      const mail = {to:recipient, subject:'KB-09 bug report — '+reference,
        body:body+'\n\nReference: '+reference, name:'KB-09 Bug Reporting'};
      if (attachment) mail.attachments = [attachment];
      MailApp.sendEmail(mail);
      cache.put(fingerprint, 'sent', 21600);
      return reportPage_(true, 'Your report was submitted. Reference: '+reference);
    } finally { lock.releaseLock(); }
  } catch (error) {
    // Never expose server exceptions, credentials, configuration, or recipients.
    return reportPage_(false, error.publicMessage || 'Submission could not be confirmed. Please save a local copy and try again later.');
  }
}
