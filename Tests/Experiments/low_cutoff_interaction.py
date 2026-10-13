"""Offline diagnosis of slow VCF MOD with a stationary VCO, not a DSP change.

Instrumented copies of the released equations record the actual cutoff command
and filter output. Compare VCO levels and 4/16x integration rates. A spectrogram
ridge is not proof of injection locking, nor is a waveform difference an alias
measurement. Hardware recordings are needed to establish analog fidelity.
"""
from pathlib import Path
import subprocess, json, wave, sys
ROOT=Path(__file__).resolve().parents[2]
# Optional local analysis libraries, isolated from the bundled runtime.
sys.path.insert(0,str(ROOT/'.tmp/dsp-analysis-packages'))
import numpy as np
from scipy import signal
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

OUT=ROOT/'artifacts/AnalogInvestigation/LowCutoffInteraction'
OUT.mkdir(parents=True,exist_ok=True)
source=(ROOT/'Tests/ReferenceDSP152/Engine.cpp').read_text()
header=(ROOT/'Tests/ReferenceDSP152/Engine.h').read_text()
lfo=(ROOT/'Tests/ReferenceDSP152/LfoRate.h').read_text()

def once(s,a,b):
    assert s.count(a)==1,a
    return s.replace(a,b)

for factor in (4,16):
    folder=OUT/f'r{factor}';folder.mkdir(exist_ok=True)
    ns=f'sh09_diag{factor}'
    h=header.replace('namespace sh09 {',f'namespace {ns} {{')
    h=once(h,'oversamplingFactor=4',f'oversamplingFactor={factor}')
    h=once(h,'firLength=513, alignment=256',f'firLength={128*factor+1}, alignment={64*factor}')
    h=once(h,'(firLength+3)/4','(firLength+oversamplingFactor-1)/oversamplingFactor')
    h=once(h,'private:','double diagnosticCutoff=0,diagnosticFilter=0;\nprivate:')
    s=source.replace('namespace sh09 {',f'namespace {ns} {{')
    s=once(s,'fc=.115','fc=.46/oversamplingFactor')
    s=once(s,'j*4+os','j*oversamplingFactor+os')
    s=once(s,'inputHistory[index]*4','inputHistory[index]*oversamplingFactor')
    s=once(s,'const double coupled=','diagnosticCutoff=finalCutoff;diagnosticFilter=filtered;\n  const double coupled=')
    (folder/'Engine.h').write_text(h)
    (folder/'Engine.cpp').write_text(s)
    (folder/'LfoRate.h').write_text(lfo.replace('namespace sh09 {',f'namespace {ns} {{'))

code='\n'.join(f'#include "r{f}/Engine.cpp"' for f in (4,16))+r'''
#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>
template<class E,class C>void render(int factor){
 for(int test=0;test<2;++test)for(int level=0;level<4;++level){
  constexpr int rate=48000,seconds=16;
  E e;e.prepare(rate);C c;c.vcaMode=0;c.vcfEnv=0;c.vcoLevel=float(level)/3;c.volume=.7f;
  c.wave=1;c.age=0;c.resonance=1;c.cutoff=test==0?0.f:.2f;c.lfoRate=0;
  c.vcfMod=test==0?1.f:.6f;c.vcoMod=0;c.subLevel=0;c.extLevel=0;c.modern=0;
  e.setControls(c);e.noteOn(69,127);
  std::vector<float> data(rate*seconds*3);bool finite=true;double peak=0;
  auto begin=std::chrono::steady_clock::now();
  for(int i=0;i<rate*seconds;++i){
   const float x=e.process(0);data[i*3]=x;data[i*3+1]=float(e.diagnosticCutoff);data[i*3+2]=float(e.diagnosticFilter);
   finite&=std::isfinite(x)&&std::isfinite(e.diagnosticFilter);peak=std::max(peak,double(std::abs(x)));
  }
  const double wall=std::chrono::duration<double>(std::chrono::steady_clock::now()-begin).count();
  const auto stem="r"+std::to_string(factor)+"_case"+std::to_string(test)+"_level"+std::to_string(level);
  std::ofstream file(stem+".f32",std::ios::binary);file.write(reinterpret_cast<char*>(data.data()),data.size()*sizeof(float));
  std::cout<<factor<<","<<test<<","<<level<<","<<wall/seconds<<","<<e.solverFailures()<<","<<peak<<","<<finite<<std::endl;
 }
}
int main(){std::cout<<"factor,case,level,cpu_ratio,solver_failures,peak,finite\n";
render<sh09_diag4::Engine,sh09_diag4::Controls>(4);
render<sh09_diag16::Engine,sh09_diag16::Controls>(16);
}
'''
(OUT/'diagnostic.cpp').write_text(code)
vs=r'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat'
(OUT/'build.cmd').write_text(f'@echo off\ncall "{vs}" -arch=x64 -host_arch=x64 >nul\ncl /nologo /std:c++20 /O2 /EHsc /utf-8 diagnostic.cpp /Fe:diagnostic.exe > build.log 2>&1\nexit /b %errorlevel%\n')
if '--analyze-only' not in sys.argv:
    subprocess.run(['cmd','/c','build.cmd'],cwd=OUT,check=True)
    with (OUT/'performance.csv').open('w') as stream:
        subprocess.run([str(OUT/'diagnostic.exe')],cwd=OUT,stdout=stream,check=True)

rate=48000;rows=[]
def read(factor,test,level):
    data=np.fromfile(OUT/f'r{factor}_case{test}_level{level}.f32',dtype=np.float32).reshape(-1,3)
    assert len(data)==rate*16 and np.isfinite(data).all()
    return data

for test in (0,1):
    fig,axes=plt.subplots(4,1,figsize=(13,10),sharex=True,sharey=True,layout='constrained')
    for level in range(4):
        data=read(4,test,level);ref=read(16,test,level)
        x=data[rate:,0].astype(float);r=ref[rate:,0].astype(float)
        cutoff=data[rate:,1].astype(float)
        floor=cutoff<=5.0001
        rows.append(dict(case=test,vco_level=level/3,cutoff_min=float(cutoff.min()),cutoff_max=float(cutoff.max()),
                         floor_fraction=float(floor.mean()),max_cutoff_step_cents=float(np.max(np.abs(1200*np.log2(cutoff[1:]/cutoff[:-1])))),
                         rms=float(np.sqrt(np.mean(x*x))),rms_16x=float(np.sqrt(np.mean(r*r))),
                         difference_vs16x_db=float(10*np.log10(max(np.mean((x-r)**2),1e-30)/max(np.mean(r*r),1e-30)))))
        freqs,times,z=signal.stft(x,rate,window='hann',nperseg=4096,noverlap=3840,nfft=16384)
        mask=(freqs>=15)&(freqs<=1600)
        axes[level].pcolormesh(times+1,freqs[mask],20*np.log10(np.maximum(abs(z[mask]),1e-8)),vmin=-65,vmax=-15,shading='auto',cmap='magma')
        axes[level].plot(np.arange(len(cutoff))[::120]/rate+1,cutoff[::120],color='#58d4dd',lw=1,label='Cutoff command (not measured oscillation frequency)')
        axes[level].axhline(440,color='white',ls=':',lw=.8)
        axes[level].set(yscale='log',ylim=(15,1600),ylabel='Hz',title=f'VCO level {level/3:.2f} — dotted line: steady 440 Hz VCO')
    axes[0].legend(loc='upper right',fontsize=8)
    axes[-1].set(xlabel='Seconds',xlim=(1,16))
    fig.suptitle(f'KB-09 released equations: Authentic / sine LFO 0.2 Hz / RES maximum / case {test}\nFixed dB scale; spectra show output components, not a unique pitch estimate')
    fig.savefig(OUT/f'case{test}-level-comparison.png',dpi=130);plt.close(fig)
    audio=np.concatenate([read(4,test,level)[rate:,0] if level==0 else np.concatenate([np.zeros(rate//2),read(4,test,level)[rate:,0]]) for level in range(4)])
    with wave.open(str(OUT/f'case{test}-vco-0-33-67-100-percent.wav'),'wb') as f:
        f.setnchannels(1);f.setsampwidth(2);f.setframerate(rate);f.writeframes((np.clip(audio,-1,1)*32767).astype('<i2').tobytes())
(OUT/'measurements.json').write_text(json.dumps(rows,indent=2))
print(json.dumps(rows,indent=2))

# Inspect actual interpolated crossing periods around the stationary VCO.
# Comparing rates here avoids mistaking STFT bin spacing for pitch steps.
periods=[]
fig,axes=plt.subplots(4,1,figsize=(12,10),layout='constrained',sharex=True,sharey=True)
for level in range(4):
    for factor,colour in ((4,'#762c86'),(16,'#e39430')):
        a=read(factor,0,level);y=a[:,2].astype(float)
        i=np.flatnonzero((y[:-1]<=0)&(y[1:]>0))
        crossing=i-y[i]/(y[i+1]-y[i])
        t=(crossing[1:]+crossing[:-1])/(2*rate);f=rate/np.diff(crossing)
        axes[level].plot(t,f,color=colour,lw=.65,label=f'{factor}x, interpolated positive zero-crossing period')
        locked=(t>5.7)&(t<6.7)&(abs(f-440)<.44)
        indices=np.flatnonzero(locked)
        runs=np.split(indices,np.flatnonzero(np.diff(indices)>1)+1)
        best=max(runs,key=len)
        lo,hi=t[best[0]],t[best[-1]]
        c=a[int(lo*rate):max(int(lo*rate)+1,int(hi*rate)),1]
        periods.append(dict(factor=factor,level=level/3,near440_longest_interval_seconds=[float(lo),float(hi)],
                            cutoff_range_hz=[float(c.min()),float(c.max())],cycles=len(best)))
    axes[level].axhline(440,color='black',ls=':',lw=.7)
    axes[level].set(ylabel='Hz',title=f'VCO mixer {level/3:.2f}',ylim=(200,510),xlim=(5.7,6.9))
axes[0].legend(fontsize=8);axes[-1].set(xlabel='Seconds')
fig.suptitle('Period trace around the VCO frequency — released model at two integration rates\nA mixed waveform has no unique instantaneous pitch; zero crossings are a diagnostic, not a tuner')
fig.savefig(OUT/'near-vco-periods.png',dpi=130);plt.close(fig)
(OUT/'period-measurements.json').write_text(json.dumps(periods,indent=2))

fig,ax=plt.subplots(figsize=(8.8,4.2),layout='constrained')
for factor,level,colour,label in ((4,0,'#246f9b','VCO off, 4x'),
                                  (4,3,'#99408b','VCO maximum, 4x'),
                                  (16,3,'#dd8b20','VCO maximum, 16x')):
    a=read(factor,0,level);y=a[:,2].astype(float)
    i=np.flatnonzero((y[:-1]<=0)&(y[1:]>0));crossing=i-y[i]/(y[i+1]-y[i])
    t=(crossing[1:]+crossing[:-1])/(2*rate);f=rate/np.diff(crossing)
    ax.plot(t,f,color=colour,lw=1.2,label=label)
ax.axhline(440,color='#666666',ls=':',lw=.8)
ax.set(xlim=(5.7,6.9),ylim=(200,510),xlabel='Seconds',ylabel='Zero-crossing frequency estimate (Hz)',
       title='Model output near a steady 440 Hz VCO\nSmooth isolated sweep; flat region when the VCO is added')
ax.legend(fontsize=9,loc='lower center');ax.grid(alpha=.15)
fig.savefig(OUT/'near-vco-periods-focus.png',dpi=130);plt.close(fig)

# Verify the diagnostic instrumentation leaves the shipping 4x output intact.
check=r'''
#include "../../../Source/DSP/Engine.cpp"
#include "r4/Engine.cpp"
#include <iostream>
int main(){unsigned long long count=0,mismatches=0;
for(int test=0;test<2;++test)for(int level=0;level<4;++level){
sh09::Engine original;sh09_diag4::Engine diagnostic;original.prepare(48000);diagnostic.prepare(48000);
sh09::Controls a;sh09_diag4::Controls b;
a.vcaMode=b.vcaMode=0;a.vcfEnv=b.vcfEnv=0;a.vcoLevel=b.vcoLevel=float(level)/3;
a.resonance=b.resonance=1;a.cutoff=b.cutoff=test==0?0.f:.2f;
a.lfoRate=b.lfoRate=0;a.vcfMod=b.vcfMod=test==0?1.f:.6f;
original.setControls(a);diagnostic.setControls(b);original.noteOn(69,127);diagnostic.noteOn(69,127);
for(int i=0;i<48000;++i){++count;if(original.process(0)!=diagnostic.process(0))++mismatches;}
}
std::cout<<"samples="<<count<<" mismatches="<<mismatches<<std::endl;return mismatches?1:0;
}
'''
(OUT/'baseline.cpp').write_text(check)
(OUT/'baseline-build.cmd').write_text(f'@echo off\ncall "{vs}" -arch=x64 -host_arch=x64 >nul\ncl /nologo /std:c++20 /O2 /EHsc /utf-8 baseline.cpp /Fe:baseline.exe > baseline-build.log 2>&1\nexit /b %errorlevel%\n')
subprocess.run(['cmd','/c','baseline-build.cmd'],cwd=OUT,check=True)
with (OUT/'baseline-check.txt').open('w') as stream:
    subprocess.run([str(OUT/'baseline.exe')],cwd=OUT,stdout=stream,check=True)
