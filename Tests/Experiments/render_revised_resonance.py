"""Audition the integrated engine against the unchanged legacy path and hardware."""
from pathlib import Path
import sys,json,subprocess,hashlib
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'.tmp/dsp-analysis-packages'))
import numpy as np
from scipy.io import wavfile
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
OUT=ROOT/'artifacts/AnalogInvestigation/ResonanceIntegration';OUT.mkdir(parents=True,exist_ok=True)
OLD=ROOT/'artifacts/AnalogInvestigation/LimiterPrototypes'
hw=json.loads((ROOT/'artifacts/AnalogInvestigation/HardwareLevelMatrix/analysis.json').read_text())
baseline=json.loads((OLD/'results.json').read_text())['results'][0]
revised=next(r for r in json.loads((OLD/'UnityGain/results.json').read_text())['results'] if r['name']=='unity_gradient')
f0=hw['segments'][0]['stationary_vco_frequency_hz']
tune=65*np.log2(f0/(440*2**((53-69)/12)))
source=(ROOT/'Source/DSP/Engine.cpp').read_text()
assert source.count('const double mix=delayedFrame.source+external*delayedFrame.externalLevel*.8;')==1
source=source.replace('const double mix=delayedFrame.source+external*delayedFrame.externalLevel*.8;',
    'const double mix=diagnosticDrive*(delayedFrame.source+external*delayedFrame.externalLevel*.8);')
source=source.replace('const double filtered=filter(', 'if(diagnosticCutoff>0)finalCutoff=diagnosticCutoff;\n  const double filtered=filter(')
header=(ROOT/'Source/DSP/Engine.h').read_text().replace('private:','double diagnosticCutoff=0,diagnosticDrive=1;\nprivate:')
(OUT/'Engine.cpp').write_text(source);(OUT/'Engine.h').write_text(header)
for name in ('LfoRate.h','ResonanceModel.h'):(OUT/name).write_bytes((ROOT/'Source/DSP'/name).read_bytes())
cpp=r'''
#include "Engine.cpp"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
int main(int argc,char**argv){
 const bool revised=std::stoi(argv[1])!=0,diagnostic=std::stoi(argv[2])!=0;
 const int sr=diagnostic?96000:48000,warm=sr*6,n=sr*18;
 sh09::Engine e(revised?sh09::FilterModel::revised:sh09::FilterModel::legacy);e.prepare(sr);
 sh09::Controls c;c.modern=diagnostic?1.f:0.f;c.vcoLevel=1;c.subLevel=c.extLevel=c.vcfEnv=c.vcoMod=0;
 c.resonance=1;c.wave=1;c.vcaMode=1;c.attack=c.decay=0;c.sustain=.7f;c.volume=1;
 c.tune=diagnostic?TUNE:100*TUNE/65;c.cutoff=.58f;c.vcfMod=diagnostic?0.f:.45f;c.lfoRate=0;
 e.setControls(c);e.noteOn(53,127);e.diagnosticDrive=std::stod(argv[3]);
 std::vector<float> trajectory;
 if(diagnostic){std::ifstream f(argv[4],std::ios::binary);trajectory.resize(warm+n);f.read((char*)trajectory.data(),trajectory.size()*4);if(!f)return 2;}
 std::vector<float> y(n);
 for(int i=0;i<warm+n;++i){if(diagnostic){
  double cutoff=trajectory[size_t(i)];
  if(revised){const double fs=sr*4.0,pi=3.14159265358979323846;
   cutoff=fs/pi*std::atan(std::tan(pi*cutoff/fs)/sh09::detail::resonantPitchCorrection(1));}
  e.diagnosticCutoff=cutoff;
 }const auto v=e.process(0);if(i>=warm)y[size_t(i-warm)]=v;}
 std::ofstream f(argv[5],std::ios::binary);f.write((char*)y.data(),y.size()*4);
 std::cout<<argv[5]<<","<<e.solverFailures()<<"\n";return e.solverFailures()?3:0;
}
'''.replace('TUNE',f'{tune:.15g}f')
(OUT/'audition-render.cpp').write_text(cpp)
bench=r'''
#include "ACTUAL_SOURCE"
#include <chrono>
#include <iostream>
#include <iomanip>
volatile double sink=0;
void run(sh09::FilterModel model,int sr,int scenario,int round){
 sh09::Engine e(model);e.prepare(sr);sh09::Controls c;c.modern=0;c.vcoLevel=1;c.subLevel=c.extLevel=c.vcfEnv=c.vcoMod=0;
 c.resonance=scenario?.6f:1.f;c.wave=scenario?2.f:1.f;c.cutoff=scenario?.75f:.58f;c.vcfMod=scenario?0.f:.45f;c.lfoRate=0;
 c.vcaMode=1;c.attack=c.decay=0;c.sustain=.7f;c.volume=1;e.setControls(c);e.noteOn(53,127);
 for(int i=0;i<sr/5;++i)e.process(0);double sum=0;const auto t=std::chrono::steady_clock::now();
 for(int i=0;i<sr/2;++i){const double v=e.process(0);sum+=v*v;}
 const double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-t).count();sink=sum;
 std::cout<<(model==sh09::FilterModel::legacy?"legacy":"revised")<<","<<sr<<","<<scenario<<","<<round<<","<<std::setprecision(12)<<seconds<<","<<e.solverFailures()<<"\n";
}
int main(){for(int sr:{44100,48000,96000})for(int scenario:{0,1})for(int round=0;round<3;++round){
 if(round%2){run(sh09::FilterModel::revised,sr,scenario,round);run(sh09::FilterModel::legacy,sr,scenario,round);}
 else {run(sh09::FilterModel::legacy,sr,scenario,round);run(sh09::FilterModel::revised,sr,scenario,round);}
}return 0;}
'''.replace('ACTUAL_SOURCE',(ROOT/'Source/DSP/Engine.cpp').as_posix())
(OUT/'actual-benchmark.cpp').write_text(bench)
vs=r'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat'
(OUT/'build-audition.cmd').write_text(f'@echo off\ncall "{vs}" -arch=x64 -host_arch=x64 >nul\ncl /nologo /std:c++20 /O2 /EHsc /utf-8 audition-render.cpp /Fe:audition-render.exe > audition-build.log 2>&1\nif errorlevel 1 exit /b 1\ncl /nologo /std:c++20 /O2 /EHsc /utf-8 actual-benchmark.cpp /Fe:actual-benchmark.exe > benchmark-build.log 2>&1\nexit /b %errorlevel%\n')
subprocess.run(['cmd','/c','build-audition.cmd'],cwd=OUT,check=True)
signals={};logs=[]
for model,row,trajectory in (('legacy',baseline,OLD/'baseline/trajectory.f32'),('revised',revised,OLD/'UnityGain/unity_gradient/trajectory.f32')):
 for diagnostic in (0,1):
    stem=f'{model}-'+('matched' if diagnostic else 'actual-controls')
    p=subprocess.run([str(OUT/'audition-render.exe'),str(int(model=='revised')),str(diagnostic),str(row['input_drive'] if diagnostic else 1),str(trajectory),str(OUT/f'{stem}.f32')],text=True,capture_output=True,check=True)
    logs.append(p.stdout);y=np.fromfile(OUT/f'{stem}.f32',dtype=np.float32).astype(float)
    assert len(y)==18*(96000 if diagnostic else 48000) and np.isfinite(y).all()
    if diagnostic:y*=10**(row['fixed_gain_db']/20)
    signals[stem]=y;wavfile.write(OUT/f'{stem}.wav',96000 if diagnostic else 48000,y.astype(np.float32))
    print(stem,'complete',flush=True)
(OUT/'audition-render.log').write_text(''.join(logs))
rate,hardware=wavfile.read(hw['path']);hardware=hardware.mean(axis=1).astype(float)
wavfile.write(OUT/'sh09-reference.wav',rate,hardware[rate:7*rate].astype(np.float32))
clips=[]
for model in ('legacy','revised'):
    clip=signals[model+'-matched'][rate:7*rate].copy()
    wavfile.write(OUT/f'{model}-preview.wav',rate,clip.astype(np.float32))
    fade=np.linspace(0,1,480);clip[:480]*=fade;clip[-480:]*=fade[::-1];clips.append(clip)
ab=np.concatenate([clips[0],np.zeros(rate//2),clips[1]])
wavfile.write(OUT/'original-then-revised.wav',rate,ab.astype(np.float32))

fig,axes=plt.subplots(1,3,figsize=(16,4.4),sharex=True,sharey=True)
fig.patch.set_facecolor('#292b2d');fig.subplots_adjust(left=.055,right=.985,top=.72,bottom=.22,wspace=.085)
fig.text(.055,.93,'SH-09 / ORIGINAL KB-09 / REVISED FILTER',color='#f4f4f1',fontsize=20,weight='bold')
fig.text(.055,.85,'Six-second excerpts at the same magnification and amplitude scale',color='#d0d2d3',fontsize=12)
for ax,y,title in zip(axes,[hardware[rate:7*rate],*clips],('Your SH-09 — VCO 10','Original KB-09 — calibrated','Revised integrated filter — calibrated')):
    b=y.reshape(-1,320);t=(np.arange(len(b))+.5)*320/rate
    ax.set_facecolor('#ff373a');ax.fill_between(t,b.min(axis=1),b.max(axis=1),color='#26090a',linewidth=0)
    ax.set(xlim=(0,6),ylim=(-.5,.5),xticks=np.arange(7),yticks=[-.5,0,.5]);ax.tick_params(colors='#dedfdf',length=0,pad=6,labelsize=10)
    ax.grid(axis='x',color='#6a1517',alpha=.4);ax.set_title(title,loc='left',color='#f4f4f1',fontsize=10,pad=12)
    for spine in ax.spines.values():spine.set_color('#202123')
axes[0].set_ylabel('Amplitude',color='#dedfdf');axes[1].set_xlabel('Seconds within excerpt',color='#dedfdf')
fig.text(.055,.08,'No AGC or peak normalization. Models use fixed free-filter/relative-carrier calibration; sweep timing is approximately aligned.',color='#d0d2d3',fontsize=10)
fig.text(.055,.035,'Offline audition of the integrated source. The installed plugin is unchanged.',color='#a6a9ab',fontsize=10)
fig.savefig(OUT/'hardware-original-revised-waveforms.png',dpi=160,facecolor=fig.get_facecolor());plt.close(fig)

p=subprocess.run([str(OUT/'actual-benchmark.exe')],text=True,capture_output=True,check=True)
(OUT/'actual-benchmark.csv').write_text('model,rate,scenario,round,seconds,failures\n'+p.stdout)
times=[]
for line in p.stdout.splitlines():
 model,sr,scenario,round,seconds,failures=line.split(',');assert int(failures)==0
 times.append(dict(model=model,rate=int(sr),scenario=int(scenario),round=int(round),seconds=float(seconds)))
summary=[]
for sr in (44100,48000,96000):
 for scenario in (0,1):
    med={m:float(np.median([r['seconds'] for r in times if r['model']==m and r['rate']==sr and r['scenario']==scenario])) for m in ('legacy','revised')}
    summary.append(dict(rate=sr,scenario=scenario,median_seconds=med,relative_cpu=med['revised']/med['legacy'],core_audio_time_fraction=med['revised']/.5))
notes=dict(cpu=summary,source_sha256={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in ('Source/DSP/Engine.cpp','Source/DSP/Engine.h','Source/DSP/ResonanceModel.h')},
    audition='Original legacy equations, 0.5s silence, then integrated revised filter; six seconds each. 5ms splice fades only. Separate previews have no fades or normalization.',
    hardware='User VCO 10 segment, 1-7 seconds. Aged specimen, not factory calibration.',
    comparison='Fixed free-filter level and high-filter carrier calibration, inherited from the earlier independent experiments. Actual-controls files use identical panel settings with no fitted gains.',
    cpu_caveat='Native full Engine::process spot check, excludes preparation/warmup/I/O; single machine, not a DAW dropout guarantee.')
(OUT/'audition-and-cpu.json').write_text(json.dumps(notes,indent=2));print(json.dumps(summary,indent=2))
