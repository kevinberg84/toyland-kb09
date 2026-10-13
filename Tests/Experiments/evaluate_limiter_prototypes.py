"""Held-out model stress and ordinary-patch checks for limiter prototypes.

No additional hardware fitting in stress cases. These tests estimate practical
promise and regressions, not factory fidelity or perceptual audibility.
"""
from pathlib import Path
import sys,json,subprocess,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'.tmp/dsp-analysis-packages'))
import numpy as np
from scipy import signal
from scipy.io import wavfile
import matplotlib;matplotlib.use('Agg')
import matplotlib.pyplot as plt
OUT=ROOT/'artifacts/AnalogInvestigation/LimiterPrototypes'
report=json.loads((OUT/'results.json').read_text());hw=json.loads((ROOT/'artifacts/AnalogInvestigation/HardwareLevelMatrix/analysis.json').read_text())
rows={r['name']:r for r in report['results']};names=('baseline','headroom','diode_headroom')
analyze_only='--analyze-only' in sys.argv
if not analyze_only:subprocess.run(['cmd','/c','build.cmd'],cwd=OUT,check=True)

def run(name,case,mode,value,drive,res,wave,note,capture):
    output=OUT/name/f'check-{case}.f32'
    start=time.time()
    if not analyze_only:
        result=subprocess.run([str(OUT/'prototypes.exe'),name,mode,str(value),str(drive),str(output),'6' if mode=='trajectory' else '2',str(capture),str(res),str(wave),str(note)],capture_output=True,text=True)
        with (OUT/'checks-solver.txt').open('a') as stream:stream.write(case+','+result.stdout+result.stderr)
        assert result.returncode==0,result.stdout+result.stderr
    y=np.fromfile(output,dtype=np.float32).astype(float)
    assert len(y)==int(capture*96000) and np.isfinite(y).all()
    return y,time.time()-start

def spread(y,w):
    n=round(96000*w);b=y[:len(y)//n*n].reshape(-1,n);tt=(np.arange(len(b))+.5)*w
    dd=20*np.log10(np.maximum(np.sqrt(np.mean(b*b,axis=1)),1e-15));q=np.percentile(dd[(tt>1)&(tt<len(y)/96000-1)],[10,50,90])
    return dict(spread_db=float(q[2]-q[0]),median_dbfs=float(q[1]))
def harmonics(y,f0):
    w=np.hanning(len(y));phase=2*np.pi*f0*np.arange(len(y))/96000
    amp=np.array([2*abs(np.sum(y*w*np.exp(-1j*k*phase)))/sum(w) for k in range(1,9)])
    return dict(fundamental_rms_dbfs=float(20*np.log10(amp[0]/np.sqrt(2))),h2_h8_relative_db=(20*np.log10(np.maximum(amp[1:]/amp[0],1e-15))).tolist())

stress=[];ordinary=[];raw_stress={}
for name in names:
    row=rows[name];gain=10**(row['fixed_gain_db']/20)
    for case,wave,note in (('octave_up_saw',1,65),('square',2,53)):
        y,wall=run(name,case,'trajectory',OUT/name/'trajectory.f32',row['input_drive'],1,wave,note,18)
        raw_stress[(name,case)]=y*gain
        stress.append(dict(name=name,case=case,levels={str(w):spread(y*gain,w) for w in (.05,.25,.5)},wall_seconds=wall,
                           caveat='Input/output scales frozen from original saw comparison; no fitting to these stress cases.'))
    for case,fc,res,drive,wave in (
        ('quiet_saw',500,0,.2,1),('open_saw',4000,0,1,1),
        ('resonant_saw',500,.6,1,1),('open_resonant_saw',4000,.6,1,1),('near_onset_square',1000,.8,1,2)):
        y,wall=run(name,case,'constant',fc,drive,res,wave,53,1)
        ordinary.append(dict(name=name,case=case,cutoff_command_hz=fc,resonance=res,drive=drive,wave=wave,
            rms_dbfs=float(20*np.log10(np.sqrt(np.mean(y*y)))),harmonics=harmonics(y,hw['segments'][0]['stationary_vco_frequency_hz']),wall_seconds=wall,
            caveat='Same command, drive and original output gain in all models; no hardware-fit gains applied.'))
    print(name,'held-out and ordinary checks complete',flush=True)

def free_harmonic_descriptor(y):
    i=np.flatnonzero((y[:-1]<=0)&(y[1:]>0));cross=i-y[i]/(y[i+1]-y[i]);f=96000/np.diff(cross)
    sample=np.arange(len(y));phase=2*np.pi*np.interp(sample,cross,np.arange(len(cross)))
    frequency=np.interp(sample,(cross[:-1]+cross[1:])/2,f)
    keep=(sample>96000)&(sample<len(y)-96000)&(frequency>1800)&(frequency<2600)
    phase=phase[keep];yy=y[keep]
    amps=np.array([2*abs(np.mean(yy*np.exp(-1j*k*phase))) for k in range(1,8)])
    return dict(h2_h7_percent_of_h1=(100*amps[1:]/amps[0]).tolist(),h2_h7_root_sum_square_percent=float(100*np.sqrt(np.sum(amps[1:]**2))/amps[0]),
                caveat='Cycle-phase projection over moving high-frequency portions; descriptive harmonic content, not stationary laboratory THD or a BA662 parameter fit.')
_,a=wavfile.read(hw['path']);hardware=a.mean(axis=1).astype(float)
free_descriptors={'hardware':free_harmonic_descriptor(hardware[54*96000:70*96000])}
for name in names:
    y=np.fromfile(OUT/name/'free.f32',dtype=np.float32).astype(float);free_descriptors[name]=free_harmonic_descriptor(y)

for item in ordinary:
    base=next(r for r in ordinary if r['name']=='baseline' and r['case']==item['case'])
    item['rms_change_vs_baseline_db']=item['rms_dbfs']-base['rms_dbfs']
    item['fundamental_change_vs_baseline_db']=item['harmonics']['fundamental_rms_dbfs']-base['harmonics']['fundamental_rms_dbfs']
summary=dict(stress=stress,ordinary=ordinary,free_harmonic_descriptors=free_descriptors,
    limitations='Model stress cases are held out from fitting, but not new hardware validation. Small sampled ordinary-patch set, not exhaustive regression. Native wall times include startup and file writes; not a DAW CPU benchmark. No production changes or listening judgments.')
(OUT/'checks.json').write_text(json.dumps(summary,indent=2))

fig,axes=plt.subplots(4,1,figsize=(11,10),layout='constrained',sharex=True,sharey=True)
signals=[('Your SH-09, VCO 10',hardware[:18*96000]),*[(label,np.fromfile(OUT/name/'full.f32',dtype=np.float32).astype(float)*10**(rows[name]['fixed_gain_db']/20)) for name,label in (
    ('baseline','Current equations — carrier matched'),('headroom','Prototype: increased OTA headroom'),('diode_headroom','Prototype: OTA headroom + loaded diode limiter'))]]
for ax,(label,y) in zip(axes,signals):
    f,t,z=signal.stft(signal.resample_poly(y,1,2),48000,nperseg=4096,noverlap=3584,nfft=16384,boundary=None,padded=False)
    mask=(f>90)&(f<3500);ax.pcolormesh(t,f[mask],20*np.log10(np.maximum(abs(z[mask]),1e-12)),vmin=-65,vmax=-10,cmap='magma',shading='auto')
    for k in range(1,6):ax.axhline(hw['segments'][0]['stationary_vco_frequency_hz']*k,color='#8bd1d3',ls=':',lw=.45)
    ax.set(title=label,yscale='log',ylim=(90,3500),xlim=(1,7),ylabel='Frequency (Hz)')
axes[-1].set_xlabel('Seconds; approximate phase alignment')
fig.suptitle('Output spectra: same free sweep and closely matched relative VCO level\nPrototype component scales unmeasured; aged hardware specimen; bright harmonic regions are not a unique locking detector')
fig.savefig(OUT/'prototype-spectra.png',dpi=140);plt.close(fig)

fig,axes=plt.subplots(2,1,figsize=(11,6),layout='constrained',sharex=True,sharey=True)
for ax,case in zip(axes,('octave_up_saw','square')):
    for name,color in zip(names,('#b25743','#1c8390','#7660aa')):
        y=raw_stress[(name,case)];n=4800;d=20*np.log10(np.maximum(np.sqrt(np.mean(y.reshape(-1,n)**2,axis=1)),1e-15));t=(np.arange(len(d))+.5)*.05
        metric=next(r for r in stress if r['name']==name and r['case']==case)['levels']['0.05']['spread_db']
        ax.plot(t,d,lw=.8,color=color,label=f'{name}: {metric:.2f} dB')
    ax.set(title=case.replace('_',' '),ylabel='50 ms RMS (dBFS)',ylim=(-22,-10),xlim=(1,17));ax.legend(fontsize=8);ax.grid(alpha=.2)
axes[-1].set_xlabel('Seconds; parameters frozen from original comparison')
fig.savefig(OUT/'held-out-model-stress.png',dpi=140);plt.close(fig)

# Check rare dips as well as the previously used middle-80% spread.
tails={}
tail_signals={'hardware':hardware[:18*96000],**{name:np.fromfile(OUT/name/'full.f32',dtype=np.float32).astype(float)*10**(row['fixed_gain_db']/20) for name,row in rows.items()}}
for name,y in tail_signals.items():
    tails[name]={}
    for window in (.05,.25):
        n=round(96000*window);b=y[:len(y)//n*n].reshape(-1,n);tt=(np.arange(len(b))+.5)*window
        dd=20*np.log10(np.maximum(np.sqrt(np.mean(b*b,axis=1)),1e-15));dd=dd[(tt>1)&(tt<17)]
        tails[name][str(window)]=dict(zip(('min','p1','p10','p50','p90','p99','max'),np.percentile(dd,[0,1,10,50,90,99,100]).tolist()))
(OUT/'level-tail-check.json').write_text(json.dumps(tails,indent=2))
auditions=[]
for name in names:
    y=tail_signals[name]
    wavfile.write(OUT/f'audition-{name}.wav',96000,y.astype(np.float32))
    if name in ('baseline','diode_headroom'):
        clip=y[96000:7*96000].copy();fade=np.linspace(0,1,480)
        clip[:480]*=fade;clip[-480:]*=fade[::-1];auditions.append(clip)
ab=np.concatenate([auditions[0],np.zeros(48000),auditions[1]])
assert np.isfinite(ab).all() and max(abs(ab))<1
wavfile.write(OUT/'audition-current-then-prototype.wav',96000,ab.astype(np.float32))
(OUT/'audition-notes.json').write_text(json.dumps(dict(order=['Current equations, calibrated','Headroom + loaded diode prototype, calibrated'],seconds_each=6,gap_seconds=.5,
    source_window_seconds=[1,7],sample_rate=96000,dtype='float32',gain='Fixed per-model calibration from the free sweep and carrier; no audition peak normalization or AGC.',
    fades='5 ms at each excerpt boundary solely to prevent audition splice clicks; native measurement files unchanged.',
    limitation='Offline diagnostic cutoff trajectory, not a newly built plugin. No assistant listening judgment.'),indent=2))
print(json.dumps(summary,indent=2))
