// Offline contract tests only. No Google deployment or real email is invoked.
const fs=require('node:fs'),vm=require('node:vm'),assert=require('node:assert/strict'),crypto=require('node:crypto'),path=require('node:path');
const root=path.resolve(__dirname,'..');let checks=0;
function test(name,fn){fn();checks++;console.log('PASS '+name);}
function server(){
 const state={props:{REPORT_TO:'kevinberg84@gmail.com'},cache:{},sent:[],quota:100,locked:false,fail:false};
 const context=vm.createContext({Date,Number,String,Object,Array,JSON,Error,
 PropertiesService:{getScriptProperties:()=>({getProperty:k=>state.props[k]??null,setProperty:(k,v)=>{state.props[k]=v;}})},
 CacheService:{getScriptCache:()=>({get:k=>state.cache[k]??null,put:(k,v)=>{state.cache[k]=v;}})},
 LockService:{getScriptLock:()=>({tryLock:()=>!state.locked,releaseLock:()=>{}})},
 MailApp:{getRemainingDailyQuota:()=>state.quota,sendEmail:mail=>{if(state.fail)throw Error('private recipient credentials');state.sent.push(mail);}},
 Utilities:{DigestAlgorithm:{SHA_256:'sha256'},Charset:{UTF_8:'utf8'},computeDigest:(alg,text)=>[...crypto.createHash(alg).update(text).digest()],getUuid:()=>crypto.randomUUID(),base64Decode:text=>[...Buffer.from(text,'base64')],base64Encode:bytes=>Buffer.from(bytes).toString('base64'),newBlob:(bytes,type,name)=>({bytes,type,name})},
 HtmlService:{createHtmlOutput:html=>({html,setTitle(){return this;}})}});
 vm.runInContext(fs.readFileSync(path.join(root,'integrations/bug-report/Code.gs'),'utf8'),context);
 return {...state,state,post:data=>context.doPost(event(data)).html,context};
}
function valid(){return {version:'1.0.3',os:'macOS 15',host:'Standalone',category:'Sound / MIDI',steps:'1. Play a note\n2. Release it',expected:'Silence',actual:'The note keeps playing',email:'',website:'',startedAt:String(Date.now()-10000),schema:'1',consent:'yes'};}
function event(data){const encoded=new URLSearchParams(data).toString();return {postData:{type:'application/x-www-form-urlencoded',length:Buffer.byteLength(encoded),contents:encoded},parameters:Object.fromEntries(Object.entries(data).map(([k,v])=>[k,[v]]))};}
test('valid report goes only to fixed server recipient',()=>{const s=server();assert.match(s.post(valid()),/Report submitted/);assert.equal(s.state.sent.length,1);assert.equal(s.state.sent[0].to,'kevinberg84@gmail.com');});
test('contact address cannot become a recipient or header',()=>{const s=server(),d=valid();d.email='visitor@example.org';s.post(d);assert.equal(s.state.sent[0].to,'kevinberg84@gmail.com');assert.equal(s.state.sent[0].replyTo,undefined);assert.match(s.state.sent[0].body,/visitor@example.org/);});
for(const field of ['version','os','host','category','steps','expected','actual','consent'])test('reject missing '+field,()=>{const s=server(),d=valid();delete d[field];assert.match(s.post(d),/Report not confirmed/);assert.equal(s.state.sent.length,0);});
for(const [field,max] of Object.entries({version:40,os:120,host:160,steps:3000,expected:1500,actual:3000,email:254}))test('reject overlong '+field,()=>{const s=server(),d=valid();d[field]='x'.repeat(max+1);s.post(d);assert.equal(s.state.sent.length,0);});
for(const [name,change] of [
 ['honeypot',d=>d.website='spam'],['invalid email',d=>d.email='x\nBcc:spam@example.org'],
 ['unknown category',d=>d.category='anything'],['too fast',d=>d.startedAt=String(Date.now())],
 ['stale timestamp',d=>d.startedAt=String(Date.now()-31*86400000)],['recipient injection',d=>d.to='attacker@example.org'],
 ['unsupported schema',d=>d.schema='2'],['control characters',d=>d.steps='abc\x00def']])
 test('reject '+name,()=>{const s=server(),d=valid();change(d);s.post(d);assert.equal(s.state.sent.length,0);});
test('duplicate field cannot bypass validation',()=>{const s=server(),e=event(valid());e.parameters.steps.push('injected');s.context.doPost(e);assert.equal(s.state.sent.length,0);});
test('reject oversized body and unsupported content type',()=>{const s=server();for(const modify of [e=>e.postData.length=4500001,e=>e.postData.type='application/json']){const e=event(valid());modify(e);s.context.doPost(e);}assert.equal(s.state.sent.length,0);});
test('deduplicate identical reports without another email',()=>{const s=server(),d=valid();s.post(d);assert.match(s.post(d),/already submitted/);assert.equal(s.state.sent.length,1);});
test('cap global ten-minute allowance',()=>{const s=server();for(let i=0;i<6;i++){const d=valid();d.actual+=' '+i;s.post(d);}assert.equal(s.state.sent.length,5);});
test('cap daily allowance',()=>{const s=server();s.state.props.RATE_STATE=JSON.stringify({day:new Date().toISOString().slice(0,10),daily:30,start:Date.now(),recent:0});s.post(valid());assert.equal(s.state.sent.length,0);});
test('quota exhaustion and lock contention fail without mail',()=>{for(const field of ['quota','locked']){const s=server();s.state[field]=field==='quota'?0:true;s.post(valid());assert.equal(s.state.sent.length,0);}});
test('missing server configuration fails clearly',()=>{const s=server();delete s.state.props.REPORT_TO;assert.match(s.post(valid()),/not configured/);assert.equal(s.state.sent.length,0);});
test('backend exception does not leak configuration',()=>{const s=server();s.state.fail=true;const html=s.post(valid());assert.doesNotMatch(html,/credentials|kevinberg84@gmail.com/);assert.match(html,/not confirmed/);});
test('HTML escaping protects confirmation messages',()=>{const s=server();assert.equal(s.context.escapeReportHtml_('<script>"&\''),'&lt;script&gt;&quot;&amp;&#39;');});
test('GET cannot send reports',()=>{const s=server();s.context.doGet();assert.equal(s.state.sent.length,0);});
function screenshot(){return {shotName:'problem.png',shotType:'image/png',shotData:Buffer.from([137,80,78,71,13,10,26,10,0,0,0,0]).toString('base64')};}
test('screenshot is an attachment sent only to the fixed recipient',()=>{const s=server();s.post({...valid(),...screenshot()});assert.equal(s.state.sent.length,1);assert.equal(s.state.sent[0].attachments.length,1);assert.equal(s.state.sent[0].attachments[0].name,'KB-09-screenshot.png');});
test('reject forged screenshot MIME or signature',()=>{for(const change of [d=>d.shotType='text/html',d=>d.shotData=Buffer.from('<script>').toString('base64')]){const s=server(),d={...valid(),...screenshot()};change(d);s.post(d);assert.equal(s.state.sent.length,0);}});
test('reject an attachment larger than two MB after decoding',()=>{const s=server(),d={...valid(),...screenshot()},bytes=Buffer.alloc(2097153);Buffer.from(d.shotData,'base64').copy(bytes);d.shotData=bytes.toString('base64');s.post(d);assert.equal(s.state.sent.length,0);});
test('different screenshots are distinct reports',()=>{const s=server(),d={...valid(),...screenshot()};s.post(d);d.shotData=Buffer.from([137,80,78,71,13,10,26,10,0,0,0,1]).toString('base64');s.post(d);assert.equal(s.state.sent.length,2);});

function frontend(endpoint){
 const listeners={},fields=valid(),clicked=[],nodes={};let savedBlob;
 const get=id=>nodes[id]??(nodes[id]={value:'',open:false,disabled:true,textContent:'',addEventListener:(type,fn)=>listeners[id+':'+type]=fn});
 const form=get('bug-report-form');form.elements={namedItem:key=>({get value(){return fields[key]??''},set value(v){fields[key]=v;}})};
 form.getAttribute=()=>endpoint;form.reportValidity=()=>true;
 const document={getElementById:get,body:{append(){}},createElement:()=>({click(){clicked.push(this.download)},remove(){}})};
 const context=vm.createContext({document,Date,String,Number,Blob,FileReader:class {readAsDataURL(file){this.result='data:'+file.type+';base64,'+file.data;this.onload();}},URL:{createObjectURL:blob=>{savedBlob=blob;return 'blob:offline-test'},revokeObjectURL(){}},setTimeout(){}});
 vm.runInContext(fs.readFileSync(path.join(root,'docs/manual/bug-report.js'),'utf8'),context);
 return {listeners,fields,nodes,clicked,blob:()=>savedBlob};
}
test('unconfigured form prevents submission and explains offline fallback',()=>{const f=frontend('');assert.equal(f.nodes['report-send'].disabled,true);let prevented=false;f.listeners['bug-report-form:submit']({preventDefault(){prevented=true}});assert.equal(prevented,true);assert.match(f.nodes['report-status'].textContent,/not connected/);});
test('configured form uses normal POST and preserves entered answers',()=>{const f=frontend('https://script.google.com/macros/s/ExampleDeployment/exec');f.fields.startedAt=String(Date.now()-5000);let prevented=false;f.listeners['bug-report-form:submit']({preventDefault(){prevented=true}});assert.equal(prevented,false);assert.equal(f.nodes['report-send'].disabled,false);assert.equal(f.fields.actual,valid().actual);});
test('local save generates a report download and a copy fallback',()=>{const f=frontend('');f.listeners['report-save:click']();assert.equal(f.clicked.length,1);assert.match(f.nodes['report-copy'].value,/Steps to reproduce/);assert.equal(f.nodes['report-copy-details'].open,true);assert.equal(f.blob().type,'text/plain;charset=utf-8');assert.equal(f.fields.steps,valid().steps);});
test('local screenshot save includes attachment in HTML and keeps form answers',()=>{const f=frontend('');f.nodes['report-screenshot'].files=[{name:'problem.png',type:'image/png',size:12,data:screenshot().shotData}];f.listeners['report-screenshot:change']();f.listeners['report-save:click']();assert.equal(f.fields.shotData,screenshot().shotData);assert.match(f.nodes['report-copy'].value,/Screenshot: problem.png/);assert.equal(f.blob().type,'text/html;charset=utf-8');assert.match(f.clicked[0],/\.html$/);f.listeners['report-remove-screenshot:click']();assert.equal(f.fields.shotData,'');assert.equal(f.fields.steps,valid().steps);});
test('invalid screenshot blocks online submission until removed',()=>{const f=frontend('https://script.google.com/macros/s/ExampleDeployment/exec');f.nodes['report-screenshot'].files=[{name:'bad.svg',type:'image/svg+xml',size:99}];f.listeners['report-screenshot:change']();assert.equal(f.nodes['report-send'].disabled,true);f.listeners['report-remove-screenshot:click']();assert.equal(f.nodes['report-send'].disabled,false);});
test('source manual is POST/new-tab and never contains server address',()=>{const source=fs.readFileSync(path.join(root,'docs/manual/index.html'),'utf8');assert.match(source,/method="post" target="_blank" rel="noopener noreferrer"/);assert.doesNotMatch(source,/kevinberg84@gmail\.com/);assert.doesNotMatch(fs.readFileSync(path.join(root,'docs/manual/bug-report.js'),'utf8'),/\bfetch\s*\(/);assert.ok(source.indexOf('<h3>Mac AU')<source.indexOf('<h3>Windows VST3'));});
console.log(`${checks} offline checks passed. Live Google authorization, deployment and email delivery are not tested.`);
