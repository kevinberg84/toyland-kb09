"""Diagnostic: run unchanged engine over an approximately hardware-like band.

Fits the dominant upper spectral component, NOT an isolated hardware oscillator
or a component-accurate calibration. Maximum model resonance is an assumption.
No production DSP edits. Per-recording display centering is not audio AGC.
"""
from pathlib import Path
import argparse,sys,json,subprocess,hashlib
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'.tmp/dsp-analysis-packages'))
import numpy as np
from scipy import signal,optimize
from scipy.io import wavfile
import matplotlib;matplotlib.use('Agg')
import matplotlib.pyplot as plt
parser=argparse.ArgumentParser();parser.add_argument('--input',type=Path,required=True)
args=parser.parse_args();out=ROOT/'artifacts/AnalogInvestigation/HardwareComparisons';out.mkdir(exist_ok=True)
carriers=json.loads((out/'carrier-levels.json').read_text())

def track(path,f0):
    sr,x=wavfile.read(path);x=x.mean(axis=1).astype(np.float64)
    x=signal.resample_poly(x,1,2)
    f,t,z=signal.stft(x,sr/2,nperseg=4096,noverlap=3584,nfft=16384,boundary=None,padded=False)
    allowed=(f>f0*1.08)&(f<1600)
    amp=abs(z);amp[~allowed]=0;k=np.argmax(amp,axis=0);cols=np.arange(len(t))
    a,b,c=np.log(np.maximum(amp[k-1,cols],1e-15)),np.log(np.maximum(amp[k,cols],1e-15)),np.log(np.maximum(amp[k+1,cols],1e-15))
    estimate=(k+.5*(a-c)/(a-2*b+c))*(f[1]-f[0])
    keep=(t>1)&(t<len(x)/(sr/2)-1)
    t=t[keep];estimate=estimate[keep];logf=np.log(estimate)
    trials=[]
    for rate in np.linspace(.12,.23,400):
        design=np.c_[np.ones(len(t)),np.sin(2*np.pi*rate*t),np.cos(2*np.pi*rate*t)]
        coef=np.linalg.lstsq(design,logf,rcond=None)[0]
        trials.append((np.mean((design@coef-logf)**2),rate,coef))
    _,rate,coef=min(trials,key=lambda q:q[0])
    def residual(p):return p[0]+p[1]*np.sin(2*np.pi*p[3]*t)+p[2]*np.cos(2*np.pi*p[3]*t)-logf
    fit=optimize.least_squares(residual,[*coef,rate],loss='soft_l1',f_scale=.02).x
    center=np.exp(fit[0]);depth=np.hypot(fit[1],fit[2]);phase=np.arctan2(fit[2],fit[1])%(2*np.pi)
    return dict(center_hz=float(center),log_depth=float(depth),rate_hz=float(fit[3]),phase=float(phase),
                estimated_low_hz=float(center*np.exp(-depth)),estimated_high_hz=float(center*np.exp(depth)),
                log_fit_error_rms=float(np.sqrt(np.mean(residual(fit)**2)))),(t,estimate)

fits={};traces={}
for instrument in ('SH-09','KB-09'):
    f0=next(r['inferred_stationary_carrier_hz'] for r in carriers if r['number']==1 and r['instrument']==instrument)
    fits[instrument],traces[instrument]=track(args.input/f'1 {instrument} Sweep.wav',f0)
target=fits['SH-09'];f0=next(r['inferred_stationary_carrier_hz'] for r in carriers if r['number']==1 and r['instrument']=='SH-09')
note=53;modern_tune=1200*np.log2(f0/(440*2**((note-69)/12)))*65/1200
cutoff=np.log((target['center_hz']/.866)/(2**((note-60)/12))/10)/np.log(2000)
mod=target['log_depth']/np.log(2)/8
def lfo(value):return .2*125**value*np.exp(-np.log(4)*max(0,1-value/.2)**2)
rate_control=optimize.brentq(lambda x:lfo(x)-target['rate_hz'],0,1)
advance=(target['phase']/(2*np.pi)+1)/target['rate_hz']
controls=dict(note=note,stored_tune=float(modern_tune),cutoff=float(cutoff),vcfMod=float(mod),
              lfoRate=float(rate_control),advance_seconds=float(advance),modern=1,resonance=1,sustain=.7)
engine=(ROOT/'Tests/ReferenceDSP152/Engine.cpp').as_posix()
code=f'#include "{engine}"\n'+r'''
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
int main(){for(int level=0;level<=1;++level){
sh09::Engine e;e.prepare(96000);sh09::Controls c;
c.modern=1;c.age=0;c.vcoLevel=float(level);c.subLevel=c.extLevel=0;c.vcoMod=0;
c.resonance=1;c.vcfEnv=0;c.vcaMode=1;c.attack=0;c.decay=0;c.sustain=.7f;c.volume=1;
CONTROLS
e.setControls(c);e.noteOn(NOTE,127);
for(int i=0;i<int(ADVANCE*96000);++i)e.process(0);
std::vector<float>x(96000*20);for(auto&v:x)v=e.process(0);
std::ofstream f("matched-vco"+std::to_string(level)+".f32",std::ios::binary);
f.write(reinterpret_cast<char*>(x.data()),x.size()*sizeof(float));
std::cout<<"level="<<level<<" failures="<<e.solverFailures()<<std::endl;
}}
'''
code=code.replace('CONTROLS',f'c.tune={modern_tune:.15g}f;c.cutoff={cutoff:.15g}f;c.vcfMod={mod:.15g}f;c.lfoRate={rate_control:.15g}f;').replace('NOTE',str(note)).replace('ADVANCE',f'{advance:.15g}')
(out/'matched.cpp').write_text(code)
vs=r'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat'
(out/'matched-build.cmd').write_text(f'@echo off\ncall "{vs}" -arch=x64 -host_arch=x64 >nul\ncl /nologo /std:c++20 /O2 /EHsc /utf-8 matched.cpp /Fe:matched.exe > matched-build.log 2>&1\nexit /b %errorlevel%\n')
subprocess.run(['cmd','/c','matched-build.cmd'],cwd=out,check=True)
with (out/'matched-solver-check.txt').open('w') as stream:subprocess.run([str(out/'matched.exe')],cwd=out,stdout=stream,check=True)
metrics=[]
fig,axes=plt.subplots(3,1,figsize=(10,7),layout='constrained',sharex=True,sharey=True)
for ax,label,level in zip(axes,('SH-09 recording, pair 1','Unchanged model, approximate matched band, VCO off','Unchanged model, approximate matched band, VCO full'),(-1,0,1)):
    if level<0:
        sr,x=wavfile.read(args.input/'1 SH-09 Sweep.wav');x=x.mean(axis=1).astype(float)
    else:
        sr=96000;x=np.fromfile(out/f'matched-vco{level}.f32',dtype=np.float32).astype(float)
        wavfile.write(out/f'matched-vco{level}.wav',sr,x.astype(np.float32))
    assert np.isfinite(x).all()
    n=int(sr*.05);xx=x[:len(x)//n*n].reshape(-1,n);rms=np.sqrt(np.mean(xx*xx,axis=1));tt=(np.arange(len(rms))+.5)*.05
    keep=(tt>1)&(tt<19);dd=20*np.log10(np.maximum(rms,1e-12));q=np.percentile(dd[keep],[10,50,90])
    metrics.append(dict(label=label,p90_p10_db=float(q[2]-q[0]),median_rms_dbfs=float(q[1])))
    ax.plot(tt,dd-q[1],lw=1,color='#197b8c' if level<0 else '#ae4e43')
    ax.set(title=f'{label} — middle 80% range {q[2]-q[0]:.2f} dB',ylim=(-16,8),ylabel='Level change (dB)',xlim=(1,19));ax.grid(alpha=.2)
axes[-1].set(xlabel='Seconds (approximate phase matching; no audio time stretching)')
fig.suptitle('Diagnostic only: narrower modulation band in the current equations\nControls inferred from a mixed recording; maximum model RES and free-oscillation scaling assumed')
fig.savefig(out/'matched-band-level-check.png',dpi=130);plt.close(fig)
report=dict(fits=fits,controls=controls,metrics=metrics,
            source_hashes={name:hashlib.sha256((ROOT/name).read_bytes()).hexdigest() for name in ('Source/DSP/Engine.cpp','Source/DSP/Engine.h','Source/DSP/LfoRate.h')},
            limitations='Dominant upper-component sine/log fit is not hardware cutoff CV or isolated self-oscillation. Model frequency scaling .866 and max resonance assumed. Tests whether approximate range narrowing alone removes modulation, not a calibrated hardware lock-range comparison.')
(out/'matched-band-check.json').write_text(json.dumps(report,indent=2))
print(json.dumps(report,indent=2))
