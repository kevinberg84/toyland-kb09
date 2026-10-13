"""Offline current-model comparison constrained by SH-09 Test #2.

One fixed output gain is fitted only to the VCO-off recording. VCO drive is then
assessed using the high-cutoff stationary carrier band, not by flattening output
levels. Measured free-frequency shape drives a diagnostic copy's cutoff directly;
this deliberately separates the filter question from imperfect LFO matching.
No production DSP edits. Kevin confirmed maximum hardware RES; physical loop
gain equivalence between hardware and model remains unmeasured.
"""
from pathlib import Path
import sys,json,subprocess,hashlib
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'.tmp/dsp-analysis-packages'))
import numpy as np
from scipy import signal
from scipy.io import wavfile
import matplotlib;matplotlib.use('Agg')
import matplotlib.pyplot as plt
OUT=ROOT/'artifacts/AnalogInvestigation/HardwareLevelMatrix/CurrentModel';OUT.mkdir(parents=True,exist_ok=True)
hardware=json.loads((OUT.parent/'analysis.json').read_text())
off=next(r for r in hardware['segments'] if r['level']==0)
fit=off['isolated_filter_periodic_fit'];f0=next(r['stationary_vco_frequency_hz'] for r in hardware['segments'] if r['level']==10)
middle=next(r for r in hardware['segments'] if r['level']==5)
full=next(r for r in hardware['segments'] if r['level']==10)
middle_ratio=10**((middle['carrier_band_when_filter_high']['median_dbfs']-full['carrier_band_when_filter_high']['median_dbfs'])/20)
scales=[1.,.75,.5];fitted_scale=None
if '--fit-carrier' in sys.argv:
    prior=json.loads((OUT/'model-comparison.json').read_text())
    points=sorted((r['high_filter_carrier_median_dbfs'],np.log(r['scale'])) for r in prior['metrics'] if r['normalized_vco_level']==1)
    target=full['carrier_band_when_filter_high']['median_dbfs']
    assert points[0][0]<target<points[-1][0]
    fitted_scale=float(np.exp(np.interp(target,[p[0] for p in points],[p[1] for p in points])))
    scales=[fitted_scale]
note=53;tune=1200*np.log2(f0/(440*2**((note-69)/12)))*65/1200
source=(ROOT/'Tests/ReferenceDSP152/Engine.cpp').read_text();header=(ROOT/'Tests/ReferenceDSP152/Engine.h').read_text();lfo=(ROOT/'Tests/ReferenceDSP152/LfoRate.h').read_text()
def once(s,a,b):
    assert s.count(a)==1,(a,s.count(a))
    return s.replace(a,b)
h=once(header,'private:','double diagnosticCutoff=1000;\nprivate:')
s=once(source,'const double filtered=filter(','finalCutoff=diagnosticCutoff;\n  const double filtered=filter(')
(OUT/'Engine.h').write_text(h);(OUT/'Engine.cpp').write_text(s);(OUT/'LfoRate.h').write_text(lfo)
code=r'''
#include "Engine.cpp"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
sh09::Controls controls(float level){sh09::Controls c;c.modern=1;c.age=0;c.vcoLevel=level;c.subLevel=c.extLevel=0;
c.vcoMod=c.vcfEnv=c.vcfMod=0;c.resonance=1;c.vcaMode=1;c.attack=c.decay=0;c.sustain=.7f;c.volume=1;c.tune=TUNE;return c;}
int main(int argc,char**argv){const int rate=96000;
if(argc>1&&std::string(argv[1])=="calibrate"){
 for(double cutoff:{100.,500.,2000.,4000.}){
  sh09::Engine e;e.prepare(rate);e.setControls(controls(0));e.noteOn(53,127);e.diagnosticCutoff=cutoff;
  for(int i=0;i<rate*2;++i)e.process(0);
  std::vector<float>x(rate);for(auto&v:x)v=e.process(0);
  std::ofstream f("calibration_"+std::to_string(int(cutoff))+".f32",std::ios::binary);f.write((char*)x.data(),x.size()*sizeof(float));
  std::cout<<"calibration,"<<cutoff<<","<<e.solverFailures()<<std::endl;
 }return 0;
}
std::ifstream file("cutoff-trajectory.f32",std::ios::binary|std::ios::ate);auto size=file.tellg();file.seekg(0);
std::vector<float>trajectory(size_t(size)/sizeof(float));file.read((char*)trajectory.data(),size);
std::vector<float>scales{1.f,.75f,.5f};
if(argc>2&&std::string(argv[1])=="fitted")scales={std::stof(argv[2])};
for(float scale:scales)for(float level:{0.f,MIDDLE,.5f,1.f}){
 sh09::Engine e;e.prepare(rate);e.setControls(controls(level*scale));e.noteOn(53,127);
 for(int i=0;i<rate*6;++i){e.diagnosticCutoff=trajectory[i];e.process(0);}
 std::vector<float>x(rate*18);
 for(int i=0;i<int(x.size());++i){e.diagnosticCutoff=trajectory[i+rate*6];x[i]=e.process(0);}
 const auto stem="scale"+std::to_string(scale)+"_level"+std::to_string(level);
 std::ofstream f(stem+".f32",std::ios::binary);f.write((char*)x.data(),x.size()*sizeof(float));
 std::cout<<"render,"<<scale<<","<<level<<","<<e.solverFailures()<<std::endl;
}
}
'''.replace('TUNE',f'{tune:.15g}f').replace('MIDDLE',f'{middle_ratio:.10g}f')
(OUT/'matrix.cpp').write_text(code)
vs=r'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat'
(OUT/'build.cmd').write_text(f'@echo off\ncall "{vs}" -arch=x64 -host_arch=x64 >nul\ncl /nologo /std:c++20 /O2 /EHsc /utf-8 matrix.cpp /Fe:matrix.exe > build.log 2>&1\nexit /b %errorlevel%\n')

def frequency(y,rate=96000):
    i=np.flatnonzero((y[:-1]<=0)&(y[1:]>0));cross=i-y[i]/(y[i+1]-y[i]);return (cross[1:]+cross[:-1])/(2*rate),rate/np.diff(cross)
if '--analyze-only' not in sys.argv:
    subprocess.run(['cmd','/c','build.cmd'],cwd=OUT,check=True)
    if fitted_scale is None:
        with (OUT/'calibration-solver.txt').open('w') as stream:subprocess.run([str(OUT/'matrix.exe'),'calibrate'],cwd=OUT,stdout=stream,check=True)
calibration=[]
for cutoff in (100,500,2000,4000):
    y=np.fromfile(OUT/f'calibration_{cutoff}.f32',dtype=np.float32).astype(float);assert np.isfinite(y).all()
    t,f=frequency(y);keep=(t>.1)&(t<.9)
    calibration.append(dict(command_hz=cutoff,oscillation_hz=float(np.median(f[keep]))))
t=np.arange(96000*24)/96000-6
logf=np.full(len(t),fit['log_mean'])
for k,(ss,cc) in enumerate(fit['sin_cos_coefficients'],1):logf+=ss*np.sin(2*np.pi*k*fit['rate_hz']*t)+cc*np.cos(2*np.pi*k*fit['rate_hz']*t)
command=np.exp(np.interp(logf,np.log([r['oscillation_hz'] for r in calibration]),np.log([r['command_hz'] for r in calibration])))
if '--analyze-only' not in sys.argv:
    command.astype(np.float32).tofile(OUT/'cutoff-trajectory.f32')
    invocation=[str(OUT/'matrix.exe')]+(['fitted',f'{fitted_scale:.10g}'] if fitted_scale is not None else [])
    with (OUT/('render-solver-fitted.txt' if fitted_scale is not None else 'render-solver.txt')).open('w') as stream:subprocess.run(invocation,cwd=OUT,stdout=stream,check=True)

def levels(y,window):
    n=round(96000*window);b=y[:len(y)//n*n].reshape(-1,n);tt=(np.arange(len(b))+.5)*window
    dd=20*np.log10(np.maximum(np.sqrt(np.mean(b*b,axis=1)),1e-15));keep=(tt>1)&(tt<len(y)/96000-1)
    q=np.percentile(dd[keep],[10,50,90]);return tt,dd,dict(p90_p10_db=float(q[2]-q[0]),median_dbfs=float(q[1]))

def target_frequency(tt):
    lf=np.full(len(tt),fit['log_mean'])
    for k,(ss,cc) in enumerate(fit['sin_cos_coefficients'],1):lf+=ss*np.sin(2*np.pi*k*fit['rate_hz']*tt)+cc*np.cos(2*np.pi*k*fit['rate_hz']*tt)
    return np.exp(lf)
raw={};metrics=[]
for scale in scales:
    for level in (0.,middle_ratio,.5,1.):
        name=f'scale{scale:.6f}_level{level:.6f}'
        y=np.fromfile(OUT/f'{name}.f32',dtype=np.float32).astype(float);assert len(y)==96000*18 and np.isfinite(y).all();raw[(scale,level)]=y
_,_,zero=levels(raw[(scales[0],0.)],.05)
gain_db=off['levels']['0.05']['median_dbfs']-zero['median_dbfs'];gain=10**(gain_db/20)
for (scale,level),raw_y in raw.items():
    y=raw_y*gain;row=dict(scale=scale,normalized_vco_level=level,levels={})
    for w in (.05,.25,.5):_,_,m=levels(y,w);row['levels'][str(w)]=m
    tt=np.arange(len(y))/96000;analytic=signal.sosfiltfilt(signal.butter(6,8,fs=96000,output='sos'),2*y*np.exp(-2j*np.pi*f0*tt))
    ct=tt[::1920];dd=20*np.log10(np.maximum(abs(analytic[::1920])/np.sqrt(2),1e-15));estimate=target_frequency(ct)
    mask=(ct>1)&(ct<17)&(estimate>1800);row['high_filter_carrier_median_dbfs']=float(np.median(dd[mask]))
    if level==0 and scale==scales[0]:
        ft,ff=frequency(y);mask=(ft>1)&(ft<17)
        # 64 samples of output FIR group delay; cutoff override bypasses the input alignment delay.
        cents=1200*np.log2(ff[mask]/target_frequency(ft[mask]-64/96000))
        row['free_trajectory_error_cents']=dict(rms=float(np.sqrt(np.mean(cents*cents))),percentiles=np.percentile(cents,[1,50,99]).tolist(),
            observed_bounds_hz=[float(ff[mask].min()),float(ff[mask].max())])
    metrics.append(row)
report=dict(hardware_source_sha256=hardware['sha256'],source_hashes={p:hashlib.sha256((ROOT/p.replace('Source/DSP/','Tests/ReferenceDSP152/')).read_bytes()).hexdigest() for p in ('Source/DSP/Engine.cpp','Source/DSP/Engine.h','Source/DSP/LfoRate.h')},
    model_resonance=1,hardware_resonance_position=10,resonance_setting_provenance='Kevin confirmed hardware RES maximum on 13 October 2026; matching panel endpoints do not establish equal loop gain.',
    model_vca_mode='steady ENV sustain .7',vco_frequency_hz=f0,note=note,tune=tune,hardware_middle_high_carrier_ratio=middle_ratio,
    fixed_output_gain_db=gain_db,output_gain_fit='One gain from VCO-off median only; applied unchanged to every model render.',
    calibration=calibration,metrics=metrics,
    fitted_scale=fitted_scale,input_fit='If fitted, input scale interpolated from high-cutoff carrier level at VCO 10 only; output envelope variation was not a fitting target.',
    limitations='Empirical measured free-frequency fit supplies diagnostic cutoff; not a production LFO change. Current filter equations unchanged. Both RES panel settings maximum; actual loop-gain equivalence unmeasured. Source waveform assumed saw from prior baseline photos. Relative high-cutoff output carrier is not direct internal input voltage. Hardware is a poorly maintained approximately 40-year-old specimen, not a factory-calibrated target. Not a calibrated injection-locking detector.')
(OUT/('model-comparison-fitted.json' if fitted_scale is not None else 'model-comparison.json')).write_text(json.dumps(report,indent=2))

fig,axes=plt.subplots(3,1,figsize=(11,8),layout='constrained',sharex=True,sharey=True)
display_cases=((1.,0.),(1.,1.),(.5,1.)) if fitted_scale is None else ((fitted_scale,0.),(1.,1.),(fitted_scale,1.))
for ax,(scale,level) in zip(axes,display_cases):
    yy=raw.get((scale,level))
    if yy is None:yy=np.fromfile(OUT/f'scale{scale:.6f}_level{level:.6f}.f32',dtype=np.float32).astype(float)
    tt,dd,m=levels(yy*gain,.05)
    label='VCO off' if not level else f'VCO full, input scale {scale:g}'
    ax.plot(tt,dd,lw=1,color='#167e90' if not level else '#b5533c')
    ax.set(title=f"{label} — spread {m['p90_p10_db']:.2f} dB",ylabel='RMS (dBFS)',xlim=(1,17));ax.grid(alpha=.2)
axes[-1].set_xlabel('Seconds in diagnostic trajectory')
fig.suptitle('Current filter equations driven by the measured hardware sweep shape\nOne output gain calibrated from VCO off; both RES controls maximum, loop gain uncalibrated')
fig.savefig(OUT/('model-level-check-fitted.png' if fitted_scale is not None else 'model-level-check.png'),dpi=140);plt.close(fig)
if fitted_scale is not None:
    sr,a=wavfile.read(hardware['path']);yy=a.mean(axis=1).astype(float)[:96000*18]
    baseline=np.fromfile(OUT/'scale1.000000_level1.000000.f32',dtype=np.float32).astype(float)*gain
    fitted=raw[(fitted_scale,1.)]*gain
    fig,axes=plt.subplots(3,1,figsize=(11,8.5),layout='constrained',sharex=True,sharey=True)
    for ax,(label,y) in zip(axes,[('Your SH-09 — VCO 10',yy),('Current filter equations — original drive',baseline),('Current filter equations — relative VCO level matched',fitted)]):
        f,tt,z=signal.stft(signal.resample_poly(y,1,2),48000,nperseg=8192,noverlap=7680,nfft=16384,boundary=None,padded=False)
        mask=(f>=90)&(f<=3500)
        ax.pcolormesh(tt,f[mask],20*np.log10(np.maximum(abs(z[mask]),1e-12)),vmin=-65,vmax=-10,cmap='magma',shading='auto')
        for k in (1,2,3,4,5):ax.axhline(k*f0,color='#8ed1d6',ls=':',lw=.5)
        ax.set(title=label,yscale='log',ylim=(90,3500),ylabel='Frequency (Hz)',xlim=(1,7))
    axes[-1].set_xlabel('Seconds; approximate phase alignment')
    fig.suptitle('Controlled diagnostic using the measured free sweep shape\nBoth RES controls maximum; saw assumed; one fixed output gain from VCO off.\nAged hardware specimen, loop gain uncalibrated; dotted lines mark VCO harmonics.')
    fig.savefig(OUT/'hardware-model-spectra.png',dpi=140);plt.close(fig)
print(json.dumps(dict(calibration=calibration,output_gain_db=gain_db,middle_ratio=middle_ratio,metrics=metrics),indent=2))
