"""Direct hardware waveform view and native timed-loop prototype checks.

No production edits. Uses existing isolated engine copies. Timings exclude
preparation, warmup and file writes; they are not a DAW dropout guarantee.
"""
from pathlib import Path
import sys, json, subprocess
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'.tmp/dsp-analysis-packages'))
import numpy as np
from scipy.io import wavfile
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
OUT=ROOT/'artifacts/AnalogInvestigation/LimiterPrototypes'
report=json.loads((OUT/'results.json').read_text())
rows={r['name']:r for r in report['results']}
hw=json.loads((ROOT/'artifacts/AnalogInvestigation/HardwareLevelMatrix/analysis.json').read_text())
sr,a=wavfile.read(hw['path'])
hardware=a.mean(axis=1)[sr:7*sr]
audio=[hardware]
for name in ('baseline','diode_headroom'):
    y=np.fromfile(OUT/name/'full.f32',dtype=np.float32)
    audio.append(y[sr:7*sr]*10**(rows[name]['fixed_gain_db']/20))
assert all(len(y)==6*sr and np.isfinite(y).all() for y in audio)
fig,axes=plt.subplots(3,1,figsize=(13,8),sharex=True,sharey=True)
fig.patch.set_facecolor('#292b2d')
fig.subplots_adjust(left=.075,right=.975,top=.84,bottom=.12,hspace=.55)
fig.text(.075,.95,'SH-09 / CURRENT KB-09 / PROPOSED FILTER',color='#f4f4f1',fontsize=19,weight='bold')
fig.text(.075,.895,'Six-second excerpts, common time and amplitude scales',color='#d0d2d3',fontsize=12)
for ax,y,title in zip(axes,audio,('Your SH-09 — VCO 10','Current KB-09 equations — calibrated','Proposed headroom + limiter — calibrated')):
    b=y.reshape(-1,320);t=(np.arange(len(b))+.5)*320/sr
    ax.set_facecolor('#ff373a')
    ax.fill_between(t,b.min(axis=1),b.max(axis=1),color='#26090a',linewidth=0)
    ax.set(xlim=(0,6),ylim=(-.5,.5),yticks=[-.5,0,.5],xticks=np.arange(7))
    ax.tick_params(colors='#dedfdf',length=0,pad=5)
    ax.grid(axis='x',color='#6a1517',alpha=.4)
    ax.set_title(title,loc='left',color='#f4f4f1',fontsize=12,pad=9)
    for spine in ax.spines.values():spine.set_color('#202123')
axes[-1].set_xlabel('Seconds within excerpt',color='#dedfdf')
fig.text(.075,.045,'No peak normalization or AGC. Model gains calibrated to the free filter and high-filter VCO band.',color='#d0d2d3',fontsize=10)
fig.text(.075,.02,'Sweep timing is approximately aligned; waveform phases differ. Offline proposal, not an installed plugin.',color='#a6a9ab',fontsize=10)
fig.savefig(OUT/'hardware-current-proposed-waveforms.png',dpi=160,facecolor=fig.get_facecolor())
plt.close(fig)

names=('baseline','headroom','diode_headroom')
cpp='\n'.join(f'#include "{n}/Engine.cpp"' for n in names)+r'''
#include <chrono>
#include <iostream>
#include <iomanip>
volatile double benchmarkSink=0;
template<class E,class C>void bench(const char* name,int rate,int scenario,int round){
 E e;e.prepare(rate);C c;c.modern=0;c.age=0;c.vcoLevel=1;c.subLevel=c.extLevel=0;
 c.vcfMod=c.vcfEnv=c.vcoMod=0;c.resonance=scenario?0.6f:1.f;
 c.wave=scenario?2:1;c.vcaMode=1;c.attack=c.decay=0;c.sustain=.7f;c.volume=1;
 e.setControls(c);e.noteOn(53,127);e.diagnosticDrive=1;
 // Identical controls and unit drive; timings are not calibration-fit output tests.
 for(int i=0;i<rate/5;++i)e.process(0);
 const int n=rate/2;double sum=0,peak=0;
 const auto begin=std::chrono::steady_clock::now();
 for(int i=0;i<n;++i){
  e.diagnosticCutoff=scenario?1000:400*std::exp2(2*std::sin(6.283185307179586*.2*i/rate));
  const double v=e.process(0);sum+=v*v;peak=std::max(peak,std::abs(v));
 }
 const double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-begin).count();
 benchmarkSink=sum;
 std::cout<<name<<","<<rate<<","<<scenario<<","<<round<<","<<std::setprecision(12)<<seconds<<","<<e.solverFailures()<<","<<peak<<"\n";
}
int main(){for(int rate:{44100,48000,96000})for(int scenario:{0,1})for(int round=0;round<3;++round){
 // Rotate order to reduce systematic warmup/order bias.
 if(round==0){CALL0 CALL1 CALL2}
 if(round==1){CALL1 CALL2 CALL0}
 if(round==2){CALL2 CALL0 CALL1}
}return 0;}
'''
for i,name in enumerate(names):
    cpp=cpp.replace(f'CALL{i}',f'bench<proto_{name}::Engine,proto_{name}::Controls>("{name}",rate,scenario,round);')
(OUT/'tradeoff-bench.cpp').write_text(cpp)
vs=r'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat'
(OUT/'build-tradeoff-bench.cmd').write_text(f'@echo off\ncall "{vs}" -arch=x64 -host_arch=x64 >nul\ncl /nologo /std:c++20 /O2 /EHsc /utf-8 tradeoff-bench.cpp /Fe:tradeoff-bench.exe > build-tradeoff-bench.log 2>&1\nexit /b %errorlevel%\n')
subprocess.run(['cmd','/c','build-tradeoff-bench.cmd'],cwd=OUT,check=True)
p=subprocess.run([str(OUT/'tradeoff-bench.exe')],cwd=OUT,text=True,capture_output=True,check=True)
(OUT/'tradeoff-bench.csv').write_text('name,rate,scenario,round,wall_seconds,solver_failures,peak\n'+p.stdout)
timings=[]
for line in p.stdout.splitlines():
    name,rate,scenario,round,seconds,failures,peak=line.split(',')
    assert int(failures)==0 and np.isfinite(float(peak))
    timings.append(dict(name=name,rate=int(rate),scenario=int(scenario),round=int(round),wall_seconds=float(seconds),peak=float(peak)))
summary=[]
for rate in (44100,48000,96000):
    for scenario in (0,1):
        med={name:float(np.median([r['wall_seconds'] for r in timings if r['name']==name and r['rate']==rate and r['scenario']==scenario])) for name in names}
        summary.append(dict(rate=rate,scenario='resonant_square' if scenario else 'maximum_resonance_sweep',median_seconds=med,
            relative_to_baseline={name:med[name]/med['baseline'] for name in names},fraction_of_audio_duration={name:med[name]/.5 for name in names}))
result=dict(timing_summary=summary,measurements=timings,
    caveat='Single-machine native CPU spot check, full process loop with diagnostic cutoff override. Preparation, warmup and file I/O excluded. Not a DAW realtime deadline/dropout benchmark; not thermal/isolation-controlled. No production changes.')
(OUT/'tradeoff-bench.json').write_text(json.dumps(result,indent=2))
print(json.dumps(summary,indent=2))
