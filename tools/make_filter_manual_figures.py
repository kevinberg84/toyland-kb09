"""Publication figures from the SH-09 reference and approved KB-09 audition.

Uses recorded samples and fixed calibration gains already present in the
audition files. No automatic leveling or peak normalization. No legacy trace.
"""
from pathlib import Path
import sys,json,hashlib
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'.tmp/dsp-analysis-packages'))
import numpy as np
from scipy.io import wavfile
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
OUT=ROOT/'docs/manual/assets'
DATA=ROOT/'artifacts/AnalogInvestigation/ResonanceIntegration'
hw=json.loads((ROOT/'artifacts/AnalogInvestigation/HardwareLevelMatrix/analysis.json').read_text())
path=Path(hw['path']);assert hashlib.sha256(path.read_bytes()).hexdigest()==hw['sha256']
rate,hardware=wavfile.read(path);hardware=hardware.mean(axis=1).astype(float)[:18*rate]
sr,model=wavfile.read(DATA/'revised-matched.wav');model=model.astype(float)
assert sr==rate==96000 and len(model)==len(hardware) and np.isfinite(model).all()
plt.rcParams.update({'font.family':'DejaVu Sans','font.size':12})
fig,axes=plt.subplots(2,1,figsize=(12,5.4),sharex=True,sharey=True)
fig.patch.set_facecolor('#292b2d');fig.subplots_adjust(left=.075,right=.975,top=.74,bottom=.15,hspace=.53)
fig.text(.075,.93,'A resonant filter sweep',color='#f4f4ef',fontsize=22,weight='bold')
fig.text(.075,.855,'Steady VCO input with a slowly moving, self-oscillating filter',color='#ced1d2',fontsize=12)
for ax,y,name in zip(axes,(hardware,model),('SH-09 hardware','KB-09 model')):
    blocks=y.reshape(-1,320);time=(np.arange(len(blocks))+.5)*320/rate
    ax.set_facecolor('#ff373a');ax.fill_between(time,blocks.min(axis=1),blocks.max(axis=1),color='#26090a',linewidth=0)
    ax.set(xlim=(0,18),ylim=(-.5,.5),xticks=np.arange(0,19,3),yticks=[-.5,0,.5])
    ax.tick_params(colors='#e0e2df',length=0,pad=6);ax.grid(axis='x',color='#6a1517',alpha=.35)
    ax.set_title(name,loc='left',color='#f4f4ef',fontsize=13,pad=10)
    for spine in ax.spines.values():spine.set_color('#202123')
axes[0].set_ylabel('Amplitude',color='#e0e2df');axes[-1].set_xlabel('Seconds',color='#e0e2df')
fig.text(.075,.035,'Common amplitude and time scales. Fixed calibration gains; no automatic leveling or peak normalization.',color='#c3c8c8',fontsize=10)
fig.savefig(OUT/'vcf-sweep-comparison.png',dpi=180,facecolor=fig.get_facecolor())
fig.savefig(OUT/'vcf-sweep-comparison.svg',facecolor=fig.get_facecolor());plt.close(fig)

fig,axes=plt.subplots(1,2,figsize=(12,3.9),sharex=True,sharey=True)
fig.patch.set_facecolor('#292b2d');fig.subplots_adjust(left=.075,right=.975,top=.70,bottom=.24,wspace=.12)
fig.text(.075,.92,'The singing filter, up close',color='#f4f4ef',fontsize=22,weight='bold')
fig.text(.075,.815,'A short waveform detail from the same recordings',color='#ced1d2',fontsize=12)
starts=[]
for ax,y,name,color in zip(axes,(hardware,model),('SH-09 hardware','KB-09 model'),('#e8b46e','#78babe')):
    start=int(1.5*rate);up=np.flatnonzero((y[start:start+rate//100-1]<0)&(y[start+1:start+rate//100]>=0))
    assert len(up)>0;start+=int(up[0])+1;end=start+round(rate*.012);starts.append(start/rate)
    ax.set_facecolor('#1c2022');ax.plot(np.arange(end-start)/rate*1000,y[start:end],color=color,lw=1.15)
    ax.set(xlim=(0,12),ylim=(-.5,.5),xticks=np.arange(0,13,2),yticks=[-.5,0,.5])
    ax.tick_params(colors='#e0e2df',length=0,pad=7);ax.grid(color='#788181',alpha=.18)
    ax.set_title(name,loc='left',color='#f4f4ef',fontsize=13,pad=12)
    ax.set_xlabel('Milliseconds',color='#e0e2df')
    for spine in ax.spines.values():spine.set_color('#454b4d')
axes[0].set_ylabel('Amplitude',color='#e0e2df')
fig.text(.075,.065,'Common scales; each excerpt starts at an upward zero crossing. Fine differences in waveform shape remain.',color='#c3c8c8',fontsize=10)
fig.savefig(OUT/'vcf-waveform-detail.png',dpi=180,facecolor=fig.get_facecolor())
fig.savefig(OUT/'vcf-waveform-detail.svg',facecolor=fig.get_facecolor());plt.close(fig)
provenance=dict(hardware_sha256=hw['sha256'],model_sha256=hashlib.sha256((DATA/'revised-matched.wav').read_bytes()).hexdigest(),
    model_source_sha256=json.loads((DATA/'audition-and-cpu.json').read_text())['source_sha256'],
    rate=rate,overview_seconds=18,detail_seconds=.012,detail_start_times=starts,
    calibration='Existing fixed free-filter and relative VCO carrier calibration. Hardware VCO 10. No additional gain changes, AGC or peak normalization.',
    limitations='Aged SH-09 specimen; approximate sweep alignment, separate waveform phases. Two excerpts do not establish exact factory or component fidelity.',
    figure_files=['vcf-sweep-comparison.png','vcf-waveform-detail.png'])
(OUT/'vcf-comparison-provenance.json').write_text(json.dumps(provenance,indent=2),encoding='utf-8')
print('Created two hardware/model comparison figures; no legacy traces included.')
