"""Offline one-factor diagnostics informed by recording pair 1.

Generated copies only: no edits to Source/DSP. These are deliberately incomplete
models, NOT hardware-calibrated candidates. Controls are the approximate band fit
from match_recorded_sweep.py. Each factor is changed independently; VCO-off
renders expose changes in free oscillation that invalidate a simple RMS ranking.
"""
from pathlib import Path
import sys, json, subprocess, hashlib, csv
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'.tmp/dsp-analysis-packages'))
import numpy as np
from scipy import signal
from scipy.io import wavfile
import matplotlib;matplotlib.use('Agg')
import matplotlib.pyplot as plt

OUT=ROOT/'artifacts/AnalogInvestigation/RecordedCaseAblation'
OUT.mkdir(parents=True,exist_ok=True)
PRIOR=ROOT/'artifacts/AnalogInvestigation/HardwareComparisons'
controls=json.loads((PRIOR/'matched-band-check.json').read_text())['controls']
source=(ROOT/'Tests/ReferenceDSP152/Engine.cpp').read_text()
header=(ROOT/'Tests/ReferenceDSP152/Engine.h').read_text()
lfo=(ROOT/'Tests/ReferenceDSP152/LfoRate.h').read_text()

def once(s,a,b):
    assert s.count(a)==1,(a,s.count(a))
    return s.replace(a,b)

variants=[
    ('baseline',4,[]),
    ('linear_mixer',4,[('1.6*sat(mix/1.6)','mix')]),
    ('linear_output',4,[('1.8*sat(filtered*delayedFrame.amplitude/1.8)','filtered*delayedFrame.amplitude')]),
    ('half_input',4,[('1.6*sat(mix/1.6)','1.6*sat(.5*mix/1.6)')]),
    ('double_stage_scale',4,[('scale=1.1,feedbackScale=2.8','scale=2.2,feedbackScale=2.8')]),
    ('half_feedback_scale',4,[('scale=1.1,feedbackScale=2.8','scale=1.1,feedbackScale=1.4')]),
    ('double_feedback_scale',4,[('scale=1.1,feedbackScale=2.8','scale=1.1,feedbackScale=5.6')]),
    ('linear_stages',4,[('g*scale*sat(v)','g*scale*v'),('derivative=g*satDerivative(v)','derivative=g')]),
    ('linear_feedback',4,[('feedbackScale*sat(feedbackArg)','k*y[3]'),('k*satDerivative(feedbackArg)','k')]),
    ('baseline_16x',16,[]),
]
for name,factor,edits in variants:
    folder=OUT/name;folder.mkdir(exist_ok=True)
    ns='diag_'+name
    h=header.replace('namespace sh09 {',f'namespace {ns} {{')
    h=once(h,'oversamplingFactor=4',f'oversamplingFactor={factor}')
    h=once(h,'firLength=513, alignment=256',f'firLength={128*factor+1}, alignment={64*factor}')
    h=once(h,'(firLength+3)/4','(firLength+oversamplingFactor-1)/oversamplingFactor')
    h=once(h,'private:','double diagnosticFilter=0,diagnosticInput=0;\nprivate:')
    s=source.replace('namespace sh09 {',f'namespace {ns} {{')
    s=once(s,'fc=.115','fc=.46/oversamplingFactor')
    s=once(s,'j*4+os','j*oversamplingFactor+os')
    s=once(s,'inputHistory[index]*4','inputHistory[index]*oversamplingFactor')
    for a,b in edits:s=once(s,a,b)
    s=once(s,'const double coupled=','diagnosticFilter=filtered;diagnosticInput=mix;\n  const double coupled=')
    (folder/'Engine.h').write_text(h)
    (folder/'Engine.cpp').write_text(s)
    (folder/'LfoRate.h').write_text(lfo.replace('namespace sh09 {',f'namespace {ns} {{'))

code='\n'.join(f'#include "{name}/Engine.cpp"' for name,_,_ in variants)+r'''
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
template<class E,class C> void render(const char*name){for(int level=0;level<=1;++level){
E e;e.prepare(96000);C c;
c.modern=1;c.age=0;c.vcoLevel=float(level);c.subLevel=c.extLevel=0;c.vcoMod=0;
c.resonance=1;c.vcfEnv=0;c.vcaMode=1;c.attack=0;c.decay=0;c.sustain=.7f;c.volume=1;
CONTROLS
e.setControls(c);e.noteOn(NOTE,127);
auto begin=std::chrono::steady_clock::now();
for(int i=0;i<int(ADVANCE*96000);++i)e.process(0);
std::vector<float>x(96000*20*3);bool finite=true;
for(int i=0;i<96000*20;++i){
 x[i*3]=e.process(0);x[i*3+1]=float(e.diagnosticFilter);x[i*3+2]=float(e.diagnosticInput);
 finite&=std::isfinite(x[i*3])&&std::isfinite(x[i*3+1])&&std::isfinite(x[i*3+2]);
}
std::ofstream f(std::string(name)+"_vco"+std::to_string(level)+".f32",std::ios::binary);
f.write(reinterpret_cast<char*>(x.data()),x.size()*sizeof(float));
std::cout<<name<<","<<level<<","<<e.solverFailures()<<","<<finite<<","<<std::chrono::duration<double>(std::chrono::steady_clock::now()-begin).count()<<std::endl;
}}
int main(){std::cout<<"variant,vco,solver_failures,finite,wall_seconds\n";
CALLS
}
'''
code=code.replace('CONTROLS',f"c.tune={controls['stored_tune']:.15g}f;c.cutoff={controls['cutoff']:.15g}f;c.vcfMod={controls['vcfMod']:.15g}f;c.lfoRate={controls['lfoRate']:.15g}f;")
code=code.replace('NOTE',str(controls['note'])).replace('ADVANCE',f"{controls['advance_seconds']:.15g}")
code=code.replace('CALLS','\n'.join(f'render<diag_{name}::Engine,diag_{name}::Controls>("{name}");' for name,_,_ in variants))
(OUT/'ablation.cpp').write_text(code)
vs=r'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat'
(OUT/'build.cmd').write_text(f'@echo off\ncall "{vs}" -arch=x64 -host_arch=x64 >nul\ncl /nologo /std:c++20 /O2 /EHsc /utf-8 ablation.cpp /Fe:ablation.exe > build.log 2>&1\nexit /b %errorlevel%\n')
if '--analyze-only' not in sys.argv:
    subprocess.run(['cmd','/c','build.cmd'],cwd=OUT,check=True)
    with (OUT/'solver-check.csv').open('w') as stream:
        subprocess.run([str(OUT/'ablation.exe')],cwd=OUT,stdout=stream,check=True)

def levels(x,window):
    n=round(96000*window);blocks=x[:len(x)//n*n].reshape(-1,n)
    t=(np.arange(len(blocks))+.5)*window
    d=20*np.log10(np.maximum(np.sqrt(np.mean(blocks*blocks,axis=1)),1e-15))
    q=np.percentile(d[(t>1)&(t<19)],[10,50,90])
    return t,d,dict(p90_p10_db=float(q[2]-q[0]),median_db=float(q[1]))

metrics=[];traces={};checks={}
for name,factor,edits in variants:
    for level in (0,1):
        data=np.fromfile(OUT/f'{name}_vco{level}.f32',dtype=np.float32).reshape(-1,3).astype(float)
        assert data.shape==(1920000,3) and np.isfinite(data).all()
        if name=='baseline':
            original=np.fromfile(PRIOR/f'matched-vco{level}.f32',dtype=np.float32)
            checks[f'vco{level}_bit_identical_to_prior']=bool(np.array_equal(data[:,0],original))
            assert checks[f'vco{level}_bit_identical_to_prior']
        row=dict(variant=name,factor=factor,vco=level,edits=edits,stages={})
        for column,stage in enumerate(('output','filter','source_before_mixer')):
            row['stages'][stage]={}
            for window in (.05,.25,.5):
                t,d,m=levels(data[:,column],window)
                row['stages'][stage][str(window)]=m
                if stage=='output' and window==.05:traces[(name,level)]=(t,d)
        if level==0:
            y=data[:,1];i=np.flatnonzero((y[:-1]<=0)&(y[1:]>0))
            crossings=i-y[i]/(y[i+1]-y[i]);tt=(crossings[:-1]+crossings[1:])/192000
            f=96000/np.diff(crossings);f=f[(tt>1)&(tt<19)]
            row['free_filter_frequency_bounds_hz']=[float(f.min()),float(f.max())]
        metrics.append(row)

report=dict(controls=controls,source_hashes={p:hashlib.sha256((ROOT/p.replace('Source/DSP/','Tests/ReferenceDSP152/')).read_bytes()).hexdigest() for p in ('Source/DSP/Engine.cpp','Source/DSP/Engine.h','Source/DSP/LfoRate.h')},
            baseline_checks=checks,metrics=metrics,
            limitations='One approximate recording-1 band; maximum RES assumed. Instrumented filter/source traces take last internal sample, not FIR-decimated audio. Not alias-energy or calibrated locking measurement. Free-oscillation level/frequency change in several ablations: low output spread alone cannot select a hardware model. No production DSP edits.')
(OUT/'measurements.json').write_text(json.dumps(report,indent=2))
with (OUT/'summary.csv').open('w',newline='') as stream:
    writer=csv.writer(stream);writer.writerow(['variant','vco','output_spread_50ms_db','filter_spread_50ms_db','output_spread_250ms_db','output_median_db','free_low_hz','free_high_hz'])
    for r in metrics:
        writer.writerow([r['variant'],r['vco'],r['stages']['output']['0.05']['p90_p10_db'],r['stages']['filter']['0.05']['p90_p10_db'],r['stages']['output']['0.25']['p90_p10_db'],r['stages']['output']['0.05']['median_db'],*r.get('free_filter_frequency_bounds_hz',['',''])])
fig,axes=plt.subplots(5,2,figsize=(13,13),layout='constrained',sharex=True,sharey=True)
for ax,(name,_,_) in zip(axes.flat,variants):
    for level,color in ((0,'#167c91'),(1,'#b4533b')):
        t,d=traces[(name,level)];keep=(t>1)&(t<19);q=np.percentile(d[keep],[10,50,90])
        ax.plot(t,d-q[1],lw=.85,color=color,label=f"VCO {level}: {q[2]-q[0]:.2f} dB")
    ax.set(title=name.replace('_',' '),xlim=(1,19),ylim=(-12,6),ylabel='Median-relative level (dB)')
    ax.grid(alpha=.2);ax.legend(fontsize=8,loc='lower right')
for ax in axes[-1]:ax.set_xlabel('Seconds')
fig.suptitle('One-factor diagnostics — same control trajectory, not equally calibrated filters\n50 ms RMS; legends give middle-80% spread. Low spread alone does not establish analog fidelity.')
fig.savefig(OUT/'ablation-levels.png',dpi=130);plt.close(fig)

# A spectral crosscheck accompanies the level metric. Dotted lines mark harmonics
# of the measured stationary VCO, not detected locks. Only display gain is aligned.
manifest=json.loads((PRIOR/'recording-manifest.json').read_text())
recording_path=next(r['file'] for r in manifest if Path(r['file']).name=='1 SH-09 Sweep.wav')
sr,hardware=wavfile.read(recording_path)
hardware=hardware.mean(axis=1).astype(float)
fig,axes=plt.subplots(4,1,figsize=(11,10),layout='constrained',sharex=True,sharey=True)
cases=[('SH-09 recording, pair 1',hardware),*[(label,np.fromfile(OUT/f'{name}_vco1.f32',dtype=np.float32).reshape(-1,3)[:,0].astype(float)) for name,label in (
    ('baseline','Current model: 4x / 384 kHz internal'),('baseline_16x','Same equations: 16x / 1.536 MHz internal'),('half_input','Diagnostic only: half filter input'))]]
f0=440*2**((controls['note']-69)/12+controls['stored_tune']/65)
for ax,(label,x) in zip(axes,cases):
    _,_,rms=levels(x,.25);reference=10**(rms['median_db']/20)
    xx=signal.resample_poly(x,1,2)
    f,t,z=signal.stft(xx,48000,nperseg=8192,noverlap=7680,nfft=16384,boundary=None,padded=False)
    mask=(f>=350)&(f<=1000)
    ax.pcolormesh(t,f[mask],20*np.log10(np.maximum(abs(z[mask])/reference,1e-8)),vmin=-40,vmax=0,cmap='magma',shading='auto')
    for harmonic in (3,4,5):ax.axhline(harmonic*f0,color='#76dcdf',lw=.6,ls=':')
    ax.set(title=label,ylabel='Frequency (Hz)',ylim=(350,1000),xlim=(1,19))
axes[-1].set_xlabel('Seconds; approximate alignment only')
fig.suptitle('Output spectra in the approximately matched band\nDotted lines: VCO harmonics. Brightest component is not a unique filter-pitch measurement.\nDisplay levels relative to each signal’s median RMS; no audio gain processing.')
fig.savefig(OUT/'ablation-spectra.png',dpi=130);plt.close(fig)
print(json.dumps(checks))
print((OUT/'summary.csv').read_text())
